/*
 * The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * SphereDesigner v0.4 - creates textured spheres
 * by Vidar Madsen <vidar@prosalg.no>
 *
 * Status: Last updated 1999-09-11
 *
 * Known issues:
 * - Might crash if you click OK or Cancel before first preview is rendered
 * - Phong might look weird with transparent textures
 *
 * Todo:
 * - Saving / Loading of presets needs an overhaul
 * - Antialiasing
 * - Global controls: Gamma, ++
 * - Beautification of GUI
 * - Clean up messy source (lots of Glade remnants)
 * - (Probably more. ;-)
 */

#define PLUG_IN_NAME "SphereDesigner"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "libgimp/stdplugins-intl.h"


#define RESPONSE_RESET 1

#define PREVIEWSIZE 150

/* These must be adjusted as more functionality is added */
#define MAXOBJECT 5
#define MAXLIGHT 5
#define MAXTEXTURE 20
#define MAXTEXTUREPEROBJ 20
#define MAXNORMAL 20
#define MAXNORMALPEROBJ 20
#define MAXATMOS 1
#define MAXCOLPERGRADIENT 5

static void query (void);
static void run   (const gchar      *name,
		   gint              nparams,
		   const GimpParam  *param,
		   gint             *nreturn_vals,
		   GimpParam       **return_vals);

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,	  /* init_proc  */
  NULL,   /* quit_proc  */
  query,  /* query_proc */
  run,	  /* run_proc   */
};

enum
{
  TRIANGLE,
  DISC,
  PLANE,
  SPHERE,
  CYLINDER,
  LIGHT
};

enum
{
  SOLID,
  CHECKER,
  MARBLE,
  LIZARD,
  IMAGE,
  PHONG,
  REFLECTION,
  REFRACTION,
  PERLIN,
  WOOD,
  TRANSPARENT,
  SPIRAL,
  SPOTS,
  SMOKE
};

enum
{
  PERSPECTIVE,
  ORTHOGONAL,
  FISHEYE
};

enum
{
  FOG
};

enum
{
  TYPE,
  TEXTURE,
  NUM_COLUMNS
};


/* World-flags */
#define SMARTAMBIENT 0x00000001

/* Object-flags */
#define NOSHADOW   0x00000001

/* Texture-flags */
#define GRADIENT   0x00000001

typedef struct
{
  gshort  xsize, ysize;
  guchar *rgb;
} image;

typedef struct
{
  gshort  	numcol;
  gdouble 	pos[MAXCOLPERGRADIENT];
  GimpVector4	color[MAXCOLPERGRADIENT];
} gradient;

typedef struct
{
  gint     	majtype;
  gint     	type;
  gulong   	flags;
  GimpVector4   color1, color2;
  gradient 	gradient;
  GimpVector4   ambient, diffuse;
  gdouble  	oscale;
  GimpVector4	scale, translate, rotate;
  image    	image;
  GimpVector4   reflection;
  GimpVector4   refraction;
  GimpVector4   transparent;
  gdouble  	ior;
  GimpVector4   phongcolor;
  gdouble  	phongsize;
  gdouble  	amount;
  gdouble  	exp;
  GimpVector4   turbulence;
} texture;

typedef struct
{
  gshort  type;
  gdouble density;
  GimpVector4  color;
  gdouble turbulence;
} atmos;

typedef struct
{
  gshort  type;
  gulong  flags;
  gshort  numtexture;
  texture texture[MAXTEXTUREPEROBJ];
  gshort  numnormal;
  texture normal[MAXNORMALPEROBJ];
} common;

typedef struct
{
  common com;
  GimpVector4 a, b, c;
} triangle;

typedef struct
{
  common	com;
  GimpVector4	a;
  gdouble	b, r;
} disc;

typedef struct
{
  common	com;
  GimpVector4	a;
  gdouble	r;
} sphere;

typedef struct
{
  common 	com;
  GimpVector4 	a, b, c;
} cylinder;

typedef struct
{
  common  	com;
  GimpVector4  	a;
  gdouble 	b;
} plane;

typedef struct
{
  common 	com;
  GimpVector4 	color;
  GimpVector4 	a;
} light;

typedef struct
{
  GimpVector4  	v1, v2;
  gshort  	inside;
  gdouble 	ior;
} ray;

typedef union
{
  common   com;
  triangle tri;
  disc     disc;
  plane    plane;
  sphere   sphere;
  cylinder cylinder;
} object;


struct world_t
{
  gint    numobj;
  object  obj[MAXOBJECT];
  gint    numlight;
  light   light[MAXLIGHT];
  gint    numtexture;
  texture texture[MAXTEXTURE];
  gulong  flags;
  gshort  quality;
  gdouble smartambient;
  gshort  numatmos;
  atmos   atmos[MAXATMOS];
};

struct camera_t
{
  GimpVector4 location, lookat, up, right;
  short  type;
  double fov, tilt;
};

static GtkWidget *drawarea = NULL;

static guchar img[PREVIEWSIZE * PREVIEWSIZE * 3];

static gint running = FALSE;

static sphere s;

struct textures_t
{
  gint   index;
  gchar *s;
  glong  n;
};

struct textures_t textures[] =
{
  { 0, N_("Solid"),   SOLID   },
  { 1, N_("Checker"), CHECKER },
  { 2, N_("Marble"),  MARBLE  },
  { 3, N_("Lizard"),  LIZARD  },
  { 4, N_("Phong"),   PHONG   },
  { 5, N_("Noise"),   PERLIN  },
  { 6, N_("Wood"),    WOOD    },
  { 7, N_("Spiral"),  SPIRAL  },
  { 8, N_("Spots"),   SPOTS   },
  { 0, NULL,          0       }
};

struct
{
  gint solid, phong, light;
}
settings = { 1, 1, 1 };


static inline void vset      (GimpVector4  *v,
                              gdouble       a,
                              gdouble       b,
                              gdouble       c);
static void    restartrender (void);
static void    drawcolor1    (GtkWidget    *widget);
static void    drawcolor2    (GtkWidget    *widget);
static void    render        (void);
static void    realrender    (GimpDrawable *drawable);
static void    fileselect    (gint          action,
                              GtkWidget    *parent);
static gint    traceray      (ray          *r,
                              GimpVector4  *col,
                              gint          level,
			      gdouble       imp);
static gdouble turbulence    (gdouble      *point,
                              gdouble       lofreq,
                              gdouble       hifreq);


#define COLORBUTTONWIDTH  30
#define COLORBUTTONHEIGHT 20

static GtkTreeView *texturelist = NULL;

static GtkObject *scalexscale, *scaleyscale, *scalezscale;
static GtkObject *rotxscale, *rotyscale, *rotzscale;
static GtkObject *posxscale, *posyscale, *poszscale;
static GtkObject *scalescale;
static GtkObject *turbulencescale;
static GtkObject *amountscale;
static GtkObject *expscale;
static GtkWidget *typemenu;
static GtkWidget *texturemenu;

#define DOT(a,b) (a[0] * b[0] + a[1] * b[1] + a[2] * b[2])

#define B 256

static gint      p[B + B + 2];
static gdouble   g[B + B + 2][3];
static gboolean  start = TRUE;
static GRand    *gr;


static void
init (void)
{
  gint i, j, k;
  gdouble v[3], s;

  /* Create an array of random gradient vectors uniformly on the unit sphere */

  gr = g_rand_new ();
  g_rand_set_seed (gr, 1);    /* Use static seed, to get reproducable results */

  for (i = 0; i < B; i++)
    {
      do
	{		      /* Choose uniformly in a cube */
	  for (j = 0; j < 3; j++)
	    v[j] = g_rand_double_range (gr, -1, 1);
	  s = DOT (v, v);
	}
      while (s > 1.0);	      /* If not in sphere try again */
      s = sqrt (s);
      for (j = 0; j < 3; j++) /* Else normalize */
	g[i][j] = v[j] / s;
    }

/* Create a pseudorandom permutation of [1..B] */

  for (i = 0; i < B; i++)
    p[i] = i;
  for (i = B; i > 0; i -= 2)
    {
      k = p[i];
      p[i] = p[j = g_rand_int_range (gr, 0, B)];
      p[j] = k;
    }

  /* Extend g and p arrays to allow for faster indexing */

  for (i = 0; i < B + 2; i++)
    {
      p[B + i] = p[i];
      for (j = 0; j < 3; j++)
	g[B + i][j] = g[i][j];
    }
}

#define setup(i,b0,b1,r0,r1) \
        t = vec[i] + 10000.; \
        b0 = ((int)t) & (B-1); \
        b1 = (b0+1) & (B-1); \
        r0 = t - (int)t; \
        r1 = r0 - 1.;


static gdouble
noise3 (gdouble * vec)
{
  gint    bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
  gdouble rx0, rx1, ry0, ry1, rz0, rz1, *q, sx, sy, sz, a, b, c, d, t, u, v;
  gint    i, j;

  if (start)
    {
      start = FALSE;
      init ();
    }

  setup (0, bx0, bx1, rx0, rx1);
  setup (1, by0, by1, ry0, ry1);
  setup (2, bz0, bz1, rz0, rz1);

  i = p[bx0];
  j = p[bx1];

  b00 = p[i + by0];
  b10 = p[j + by0];
  b01 = p[i + by1];
  b11 = p[j + by1];

#define at(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

#define surve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

  sx = surve (rx0);
  sy = surve (ry0);
  sz = surve (rz0);


  q = g[b00 + bz0];
  u = at (rx0, ry0, rz0);
  q = g[b10 + bz0];
  v = at (rx1, ry0, rz0);
  a = lerp (sx, u, v);

  q = g[b01 + bz0];
  u = at (rx0, ry1, rz0);
  q = g[b11 + bz0];
  v = at (rx1, ry1, rz0);
  b = lerp (sx, u, v);

  c = lerp (sy, a, b);		/* interpolate in y at lo x */

  q = g[b00 + bz1];
  u = at (rx0, ry0, rz1);
  q = g[b10 + bz1];
  v = at (rx1, ry0, rz1);
  a = lerp (sx, u, v);

  q = g[b01 + bz1];
  u = at (rx0, ry1, rz1);
  q = g[b11 + bz1];
  v = at (rx1, ry1, rz1);
  b = lerp (sx, u, v);

  d = lerp (sy, a, b);		/* interpolate in y at hi x */

  return 1.5 * lerp (sz, c, d);	/* interpolate in z */
}

static double
turbulence (gdouble * point, gdouble lofreq, gdouble hifreq)
{
  gdouble freq, t, p[3];

  p[0] = point[0] + 123.456;
  p[1] = point[1] + 234.567;
  p[2] = point[2] + 345.678;

  t = 0;
  for (freq = lofreq; freq < hifreq; freq *= 2.)
    {
      t += noise3 (p) / freq;
      p[0] *= 2.;
      p[1] *= 2.;
      p[2] *= 2.;
    }
  return t - 0.3;		/* readjust to make mean value = 0.0 */
}

struct camera_t camera;
struct world_t  world;

static inline void
vcopy (GimpVector4 *a, GimpVector4 *b)
{
  *a = *b;
}

static inline void
vcross (GimpVector4 *r, GimpVector4 *a, GimpVector4 *b)
{
  r->x = a->y * b->z - a->z * b->y;
  r->y = -(a->x * b->z - a->z * b->x);
  r->z = a->x * b->y - a->y * b->x;
}

static inline gdouble
vdot (GimpVector4 *a, GimpVector4 *b)
{
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline gdouble
vdist (GimpVector4 *a, GimpVector4 *b)
{
  gdouble x, y, z;

  x = a->x - b->x;
  y = a->y - b->y;
  z = a->z - b->z;

  return sqrt (x * x + y * y + z * z);
}

static inline gdouble
vlen (GimpVector4 *a)
{
  return sqrt (a->x * a->x + a->y * a->y + a->z * a->z);
}

static inline void
vnorm (GimpVector4 *a, gdouble v)
{
  gdouble d;

  d = vlen (a);
  a->x *= v / d;
  a->y *= v / d;
  a->z *= v / d;
}

static inline void
vrotate (GimpVector4 *axis, gdouble ang, GimpVector4 *vector)
{
  gdouble rad = ang / 180.0 * G_PI;
  gdouble ax  = vector->x;
  gdouble ay  = vector->y;
  gdouble az  = vector->z;
  gdouble x   = axis->x;
  gdouble y   = axis->y;
  gdouble z   = axis->z;
  gdouble c   = cos (rad);
  gdouble s   = sin (rad);
  gdouble c1  = 1.0 - c;
  gdouble xx  = c1 * x * x;
  gdouble yy  = c1 * y * y;
  gdouble zz  = c1 * z * z;
  gdouble xy  = c1 * x * y;
  gdouble xz  = c1 * x * z;
  gdouble yz  = c1 * y * z;
  gdouble sx  = s * x;
  gdouble sy  = s * y;
  gdouble sz  = s * z;

  vector->x = (xx + c) * ax + (xy + sz) * ay + (xz - sy) * az;
  vector->y = (xy - sz) * ax + (yy + c) * ay + (yz + sx) * az;
  vector->z = (xz + sy) * ax + (yz - sx) * ay + (zz + c) * az;
}

static inline void
vset (GimpVector4 *v, gdouble a, gdouble b, gdouble c)
{
  v->x = a;
  v->y = b;
  v->z = c;
  v->w = 1.0;
}

static inline void
vcset (GimpVector4 *v, gdouble a, gdouble b, gdouble c, gdouble d)
{
  v->x = a;
  v->y = b;
  v->z = c;
  v->w = d;
}

static inline void
vvrotate (GimpVector4 *p, GimpVector4 *rot)
{
  GimpVector4 axis;

  if (rot->x != 0.0)
    {
      vset (&axis, 1, 0, 0);
      vrotate (&axis, rot->x, p);
    }
  if (rot->y != 0.0)
    {
      vset (&axis, 0, 1, 0);
      vrotate (&axis, rot->y, p);
    }
  if (rot->z != 0.0)
    {
      vset (&axis, 0, 0, 1);
      vrotate (&axis, rot->z, p);
    }
}

static inline void
vsub (GimpVector4 *a, GimpVector4 *b)
{
  a->x -= b->x;
  a->y -= b->y;
  a->z -= b->z;
  a->w -= b->w;
}

static inline void
vadd (GimpVector4 *a, GimpVector4 *b)
{
  a->x += b->x;
  a->y += b->y;
  a->z += b->z;
  a->w += b->w;
}

static inline void
vneg (GimpVector4 *a)
{
  a->x = -a->x;
  a->y = -a->y;
  a->z = -a->z;
  a->w = -a->w;
}

static inline void
vmul (GimpVector4 *v, gdouble a)
{
  v->x *= a;
  v->y *= a;
  v->z *= a;
  v->w *= a;
}

static inline void
vvmul (GimpVector4 *a, GimpVector4 *b)
{
  a->x *= b->x;
  a->y *= b->y;
  a->z *= b->z;
  a->w *= b->w;
}

static inline void
vvdiv (GimpVector4 *a, GimpVector4 *b)
{
  a->x /= b->x;
  a->y /= b->y;
  a->z /= b->z;
}

static void
vmix (GimpVector4 *r, GimpVector4 *a, GimpVector4 *b, gdouble v)
{
  gdouble i = 1.0 - v;

  r->x = a->x * v + b->x * i;
  r->y = a->y * v + b->y * i;
  r->z = a->z * v + b->z * i;
  r->w = a->w * v + b->w * i;
}

static double
vmax (GimpVector4 *a)
{
  gdouble max = fabs (a->x);

  if (fabs (a->y) > max)
    max = fabs (a->y);
  if (fabs (a->z) > max)
    max = fabs (a->z);
  if (fabs (a->w) > max)
    max = fabs (a->w);

  return max;
}

#if 0
static void
vavg (GimpVector4 * a)
{
  gdouble s;

  s = (a->x + a->y + a->z) / 3.0;
  a->x = a->y = a->z = s;
}
#endif

static void
trianglenormal (GimpVector4 * n, gdouble *t, triangle * tri)
{
  triangle tmp;
  vcopy (&tmp.b, &tri->b);
  vcopy (&tmp.c, &tri->c);
  vsub (&tmp.b, &tri->a);
  vsub (&tmp.c, &tri->a);
  vset (&tmp.a, 0, 0, 0);
  vcross (n, &tmp.b, &tmp.c);
  if (t)
    *t = vdot (&tmp.b, &tmp.c);
}

static gdouble
checkdisc (ray * r, disc * disc)
{
  GimpVector4 p, *v = &disc->a;
  gdouble t, d;
  gdouble i, j, k;

  i = r->v2.x - r->v1.x;
  j = r->v2.y - r->v1.y;
  k = r->v2.z - r->v1.z;

  t = -(v->x * r->v1.x + v->y * r->v1.y + v->z * r->v1.z - disc->b) /
    (v->x * i + v->y * j + v->z * k);

  p.x = r->v1.x + i * t;
  p.y = r->v1.y + j * t;
  p.z = r->v1.z + k * t;

  d = vdist (&p, v);

  if (d > disc->r)
    t = 0.0;

  return t;
}

static gdouble
checksphere (ray * r, sphere * sphere)
{
  GimpVector4 cendir, rdir;
  gdouble dirproj, cdlensq;
  gdouble linear, constant, rsq, quadratic, discriminant;
  gdouble smallzero, solmin, solmax, tolerance = 0.001;

  vcopy (&rdir, &r->v2);
  vsub (&rdir, &r->v1);

  rsq = sphere->r * sphere->r;

  vcopy (&cendir, &r->v1);
  vsub (&cendir, &sphere->a);
  dirproj = vdot (&rdir, &cendir);
  cdlensq = vdot (&cendir, &cendir);

  if ((cdlensq >= rsq) && (dirproj > 0.0))
    return 0.0;

  linear = 2.0 * dirproj;
  constant = cdlensq - rsq;
  quadratic = vdot (&rdir, &rdir);

  smallzero = (constant / linear);
  if ((smallzero < tolerance) && (smallzero > -tolerance))
    {
      solmin = -linear / quadratic;

      if (solmin > tolerance)
	{
	  return solmin;
	  /*
	   *hits = solmin;
	   return 1;
	   */
	}
      else
	return 0.0;
    }
  discriminant = linear * linear - 4.0 * quadratic * constant;
  if (discriminant < 0.0)
    return 0.0;
  quadratic *= 2.0;
  discriminant = sqrt (discriminant);
  solmax = (-linear + discriminant) / (quadratic);
  solmin = (-linear - discriminant) / (quadratic);

  if (solmax < tolerance)
    return 0.0;

  if (solmin < tolerance)
    {
      return solmax;
      /*
       * hits = solmax;
       * return 1;
       */
    }
  else
    {
      return solmin;
      /*
       * hits++ = solmin;
       * hits = solmax;
       * return 2;
       */
    }
}

static gdouble
checkcylinder (ray * r, cylinder * cylinder)
{
  /* FIXME */
  return 0.0;
}


static gdouble
checkplane (ray * r, plane * plane)
{
  GimpVector4 *v = &plane->a;
  gdouble t;
  gdouble i, j, k;

  i = r->v2.x - r->v1.x;
  j = r->v2.y - r->v1.y;
  k = r->v2.z - r->v1.z;

  t = -(v->x * r->v1.x + v->y * r->v1.y + v->z * r->v1.z - plane->b) /
    (v->x * i + v->y * j + v->z * k);

  return t;
}

static gdouble
checktri (ray * r, triangle * tri)
{
  GimpVector4  ed1, ed2;
  GimpVector4  tvec, pvec, qvec;
  gdouble det, idet, t, u, v;
  GimpVector4 *orig, dir;

  orig = &r->v1;
  dir = r->v2;
  vsub (&dir, orig);

  ed1.x = tri->c.x - tri->a.x;
  ed1.y = tri->c.y - tri->a.y;
  ed1.z = tri->c.z - tri->a.z;
  ed2.x = tri->b.x - tri->a.x;
  ed2.y = tri->b.y - tri->a.y;
  ed2.z = tri->b.z - tri->a.z;
  vcross (&pvec, &dir, &ed2);
  det = vdot (&ed1, &pvec);

  idet = 1.0 / det;

  tvec.x = orig->x;
  tvec.y = orig->y;
  tvec.z = orig->z;
  vsub (&tvec, &tri->a);
  u = vdot (&tvec, &pvec) * idet;

  if (u < 0.0)
    return 0;
  if (u > 1.0)
    return 0;

  vcross (&qvec, &tvec, &ed1);
  v = vdot (&dir, &qvec) * idet;

  if ((v < 0.0) || (u + v > 1.0))
    return 0;

  t = vdot (&ed2, &qvec) * idet;

  return t;
}

static void
transformpoint (GimpVector4 * p, texture * t)
{
  gdouble point[3], f;

  if ((t->rotate.x != 0.0) || (t->rotate.y != 0.0) || (t->rotate.z != 0.0))
    vvrotate (p, &t->rotate);
  vvdiv (p, &t->scale);

  vsub (p, &t->translate);

  if ((t->turbulence.x != 0.0) || (t->turbulence.y != 0.0) ||
      (t->turbulence.z != 0.0))
    {
      point[0] = p->x;
      point[1] = p->y;
      point[2] = p->z;
      f = turbulence (point, 1, 256);
      p->x += t->turbulence.x * f;
      p->y += t->turbulence.y * f;
      p->z += t->turbulence.z * f;
    }
}

static void
checker (GimpVector4 *q, GimpVector4 *col, texture *t)
{
  gint   c = 0;
  GimpVector4 p;

  p = *q;
  transformpoint (&p, t);

  vmul (&p, 0.25);

  p.x += 0.00001;
  p.y += 0.00001;
  p.z += 0.00001;

  if (p.x < 0.0)
    p.x = 0.5 - p.x;
  if (p.y < 0.0)
    p.y = 0.5 - p.y;
  if (p.z < 0.0)
    p.z = 0.5 - p.z;

  if ((p.x - (gint) p.x) < 0.5)
    c ^= 1;
  if ((p.y - (gint) p.y) < 0.5)
    c ^= 1;
  if ((p.z - (gint) p.z) < 0.5)
    c ^= 1;

  *col = (c) ? t->color1 : t->color2;
}

static void
gradcolor (GimpVector4 *col, gradient *t, gdouble val)
{
  gint    i;
  gdouble d;
  GimpVector4  tmpcol;

  val = CLAMP (val, 0.0, 1.0);

  for (i = 0; i < t->numcol; i++)
    {
      if (t->pos[i] == val)
	{
	  *col = t->color[i];
	  return;
	}
      if (t->pos[i] > val)
	{
	  d = (val - t->pos[i - 1]) / (t->pos[i] - t->pos[i - 1]);
	  vcopy (&tmpcol, &t->color[i]);
	  vmul (&tmpcol, d);
	  vcopy (col, &tmpcol);
	  vcopy (&tmpcol, &t->color[i - 1]);
	  vmul (&tmpcol, 1.0 - d);
	  vadd (col, &tmpcol);
	  return;
	}
    }
  fprintf (stderr, "Error in gradient!\n");
  vset (col, 0, 1, 0);
}

static void
marble (GimpVector4 *q, GimpVector4 *col, texture *t)
{
  gdouble f;
  GimpVector4 p;

  p = *q;
  transformpoint (&p, t);

  f = sin (p.x * 4) / 2 + 0.5;
  f = pow (f, t->exp);

  if (t->flags & GRADIENT)
    gradcolor (col, &t->gradient, f);
  else
    vmix (col, &t->color1, &t->color2, f);
}

static void
lizard (GimpVector4 *q, GimpVector4 *col, texture *t)
{
  gdouble f;
  GimpVector4 p;

  p = *q;
  transformpoint (&p, t);

  f = fabs (sin (p.x * 4));
  f += fabs (sin (p.y * 4));
  f += fabs (sin (p.z * 4));
  f /= 3.0;
  f = pow (f, t->exp);

  if (t->flags & GRADIENT)
    gradcolor (col, &t->gradient, f);
  else
    vmix (col, &t->color1, &t->color2, f);
}

static void
wood (GimpVector4 *q, GimpVector4 *col, texture *t)
{
  gdouble f;
  GimpVector4 p;

  p = *q;
  transformpoint (&p, t);

  f = fabs (p.x);
  f = f - (int) f;

  f = pow (f, t->exp);

  if (t->flags & GRADIENT)
    gradcolor (col, &t->gradient, f);
  else
    vmix (col, &t->color1, &t->color2, f);
}

static void
spiral (GimpVector4 *q, GimpVector4 *col, texture *t)
{
  gdouble f;
  GimpVector4 p;

  p = *q;
  transformpoint (&p, t);

  f = fabs (atan2 (p.x, p.z) / G_PI / 2 + p.y + 99999);
  f = f - (int) f;

  f = pow (f, t->exp);

  if (t->flags & GRADIENT)
    gradcolor (col, &t->gradient, f);
  else
    vmix (col, &t->color1, &t->color2, f);
}

static void
spots (GimpVector4 *q, GimpVector4 *col, texture *t)
{
  gdouble f;
  GimpVector4 p, r;

  p = *q;
  transformpoint (&p, t);

  p.x += 10000.0;
  p.y += 10000.0;
  p.z += 10000.0;

  vset (&r, (gint) (p.x + 0.5), (gint) (p.y + 0.5), (gint) (p.z + 0.5));
  f = vdist (&p, &r);
  f = cos (f * G_PI);
  f = CLAMP (f, 0.0, 1.0);
  f = pow (f, t->exp);

  if (t->flags & GRADIENT)
    gradcolor (col, &t->gradient, f);
  else
    vmix (col, &t->color1, &t->color2, f);
}

static void
perlin (GimpVector4 * q, GimpVector4 * col, texture * t)
{
  gdouble f, point[3];
  GimpVector4  p;

  p = *q;
  transformpoint (&p, t);

  point[0] = p.x;
  point[1] = p.y;
  point[2] = p.z;

  f = turbulence (point, 1, 256) * 0.3 + 0.5;

  f = pow (f, t->exp);

  if (t->flags & GRADIENT)
    gradcolor (col, &t->gradient, f);
  else
    vmix (col, &t->color1, &t->color2, f);
}

static void
imagepixel (GimpVector4 * q, GimpVector4 * col, texture * t)
{
  GimpVector4 p;
  gint x, y;
  guchar *rgb;

  p = *q;
  transformpoint (&p, t);

  x = (p.x * t->image.xsize);
  y = (p.y * t->image.ysize);

  x = (x % t->image.xsize + t->image.xsize) % t->image.xsize;
  y = (y % t->image.ysize + t->image.ysize) % t->image.ysize;

  rgb = &t->image.rgb[x * 3 + (t->image.ysize - 1 - y) * t->image.xsize * 3];
  vset (col, rgb[0] / 255.0, rgb[1] / 255.0, rgb[2] / 255.0);
}

static void
objcolor (GimpVector4 *col, GimpVector4 *p, common *obj)
{
  gint     i;
  texture *t;
  GimpVector4   tmpcol;

  vcset (col, 0, 0, 0, 0);

  for (i = 0; i < obj->numtexture; i++)
    {
      t = &obj->texture[i];

      if (world.quality < 1)
	{
	  vadd (col, &t->color1);
	  continue;
	}

      vset (&tmpcol, 0, 0, 0);
      switch (t->type)
	{
	case SOLID:
	  vcopy (&tmpcol, &t->color1);
	  break;
	case CHECKER:
	  checker (p, &tmpcol, t);
	  break;
	case MARBLE:
	  marble (p, &tmpcol, t);
	  break;
	case LIZARD:
	  lizard (p, &tmpcol, t);
	  break;
	case PERLIN:
	  perlin (p, &tmpcol, t);
	  break;
	case WOOD:
	  wood (p, &tmpcol, t);
	  break;
	case SPIRAL:
	  spiral (p, &tmpcol, t);
	  break;
	case SPOTS:
	  spots (p, &tmpcol, t);
	  break;
	case IMAGE:
	  imagepixel (p, &tmpcol, t);
	  break;
	case PHONG:
	case REFRACTION:
	case REFLECTION:
	case TRANSPARENT:
	case SMOKE:
	  /* Silently ignore non-color textures */
	  continue;
	  break;
	default:
	  fprintf (stderr, "Warning: unknown texture %d\n", t->type);
	  break;
	}
      vmul (&tmpcol, t->amount);
      vadd (col, &tmpcol);
    }
  if (!i)
    {
      fprintf (stderr, "Warning: object %p has no textures\n", obj);
    }
}

static void
objnormal (GimpVector4 *res, common *obj, GimpVector4 *p)
{
  gint i;

  switch (obj->type)
    {
    case TRIANGLE:
      trianglenormal (res, NULL, (triangle *) obj);
      break;
    case DISC:
      vcopy (res, &((disc *) obj)->a);
      break;
    case PLANE:
      vcopy (res, &((plane *) obj)->a);
      break;
    case SPHERE:
      vcopy (res, &((sphere *) obj)->a);
      vsub (res, p);
      break;
    case CYLINDER:
      vset (res, 1, 1, 1);	/* fixme */
      break;
    default:
      fprintf (stderr, "objnormal(): Unsupported object type!?\n");
      exit (0);
    }
  vnorm (res, 1.0);

  for (i = 0; i < obj->numnormal; i++)
    {
      gint     k;
      GimpVector4   tmpcol[6];
      GimpVector4   q[6], nres;
      texture *t = &obj->normal[i];
      gdouble  nstep = 0.1;

      vset (&nres, 0, 0, 0);
      for (k = 0; k < 6; k++)
	{
	  vcopy (&q[k], p);
	}
      q[0].x += nstep;
      q[1].x -= nstep;
      q[2].y += nstep;
      q[3].y -= nstep;
      q[4].z += nstep;
      q[5].z -= nstep;

      switch (t->type)
	{
	case MARBLE:
	  for (k = 0; k < 6; k++)
	    marble (&q[k], &tmpcol[k], t);
	  break;
	case LIZARD:
	  for (k = 0; k < 6; k++)
	    lizard (&q[k], &tmpcol[k], t);
	  break;
	case PERLIN:
	  for (k = 0; k < 6; k++)
	    perlin (&q[k], &tmpcol[k], t);
	  break;
	case WOOD:
	  for (k = 0; k < 6; k++)
	    wood (&q[k], &tmpcol[k], t);
	  break;
	case SPIRAL:
	  for (k = 0; k < 6; k++)
	    spiral (&q[k], &tmpcol[k], t);
	  break;
	case SPOTS:
	  for (k = 0; k < 6; k++)
	    spots (&q[k], &tmpcol[k], t);
	  break;
	case IMAGE:
	  for (k = 0; k < 6; k++)
	    imagepixel (&q[k], &tmpcol[k], t);
	  break;
	case CHECKER:
	case SOLID:
	case PHONG:
	case REFRACTION:
	case REFLECTION:
	case TRANSPARENT:
	case SMOKE:
	  continue;
	  break;
	default:
	  fprintf (stderr, "Warning: unknown texture %d\n", t->type);
	  break;
	}

      nres.x = tmpcol[0].x - tmpcol[1].x;
      nres.y = tmpcol[2].x - tmpcol[3].x;
      nres.z = tmpcol[4].x - tmpcol[5].x;
      vadd (&nres, res);
      vnorm (&nres, 1.0);
      vmul (&nres, t->amount);
      vadd (res, &nres);
      vnorm (res, 1.0);
    }
}

/*
   Quality:
   0 = Color only
   1 = Textures
   2 = Light + Normals
   3 = Shadows
   4 = Phong
   5 = Reflection + Refraction
 */

static void
calclight (GimpVector4 * col, GimpVector4 * point, common * obj)
{
  gint i, j;
  ray r;
  gdouble d, b, a;
  GimpVector4 lcol;
  GimpVector4 norm;
  GimpVector4 pcol;

  vcset (col, 0, 0, 0, 0);

  objcolor (&pcol, point, obj);
  a = pcol.w;

  if (world.quality < 2)
    {
      vcopy (col, &pcol);
      return;
    }

  for (i = 0; i < obj->numtexture; i++)
    {
      if (obj->texture[i].type == PHONG)
	continue;
      if (obj->texture[i].type == REFLECTION)
	continue;
      if (obj->texture[i].type == REFRACTION)
	continue;
      if (obj->texture[i].type == TRANSPARENT)
	continue;
      if (obj->texture[i].type == SMOKE)
	continue;
      vcopy (&lcol, &pcol);
      vvmul (&lcol, &obj->texture[i].ambient);
      vadd (col, &lcol);
    }

  objnormal (&norm, obj, point);
  vnorm (&norm, 1.0);

  r.inside = -1;
  r.ior = 1.0;

  for (i = 0; i < world.numlight; i++)
    {
      vcopy (&r.v1, point);
      vcopy (&r.v2, &world.light[i].a);
      vmix (&r.v1, &r.v1, &r.v2, 0.9999);
      d = vdist (&r.v1, &r.v2);

      vsub (&r.v1, &r.v2);
      vnorm (&r.v1, 1.0);
      b = vdot (&r.v1, &norm);

      if (b < 0.0)
	continue;

      for (j = 0; j < obj->numtexture; j++)
	{
	  if (obj->texture[j].type == PHONG)
	    continue;
	  if (obj->texture[j].type == REFLECTION)
	    continue;
	  if (obj->texture[j].type == REFRACTION)
	    continue;
	  if (obj->texture[j].type == TRANSPARENT)
	    continue;
	  if (obj->texture[j].type == SMOKE)
	    continue;
	  vcopy (&lcol, &pcol);
	  vvmul (&lcol, &world.light[i].color);
	  vvmul (&lcol, &obj->texture[j].diffuse);
	  vmul (&lcol, b);
	  vadd (col, &lcol);
	}
    }
  col->w = a;
}

static void
calcphong (common * obj, ray * r2, GimpVector4 * col)
{
  gint    i, j, o;
  ray     r;
  gdouble d, b;
  GimpVector4  lcol;
  GimpVector4  norm;
  GimpVector4  pcol;
  gdouble ps;

  vcopy (&pcol, col);

  vcopy (&norm, &r2->v2);
  vsub (&norm, &r2->v1);
  vnorm (&norm, 1.0);

  r.inside = -1;
  r.ior = 1.0;

  for (i = 0; i < world.numlight; i++)
    {
      vcopy (&r.v1, &r2->v1);
      vcopy (&r.v2, &world.light[i].a);
      vmix (&r.v1, &r.v1, &r.v2, 0.9999);
      d = vdist (&r.v1, &r.v2);

      o = traceray (&r, NULL, -1, 1.0);
      if (o)
	{
	  continue;
	}

      /* OK, light is visible */

      vsub (&r.v1, &r.v2);
      vnorm (&r.v1, 1.0);
      b = -vdot (&r.v1, &norm);

      for (j = 0; j < obj->numtexture; j++)
	{
	  if (obj->texture[j].type != PHONG)
	    continue;

	  ps = obj->texture[j].phongsize;

	  if (b < (1 - ps))
	    continue;
	  ps = (b - (1 - ps)) / ps;

	  vcopy (&lcol, &obj->texture[j].phongcolor);
	  vvmul (&lcol, &world.light[i].color);
	  vmul (&lcol, ps);
	  vadd (col, &lcol);
	}
    }
}

static int
traceray (ray * r, GimpVector4 * col, gint level, gdouble imp)
{
  gint     i, b = -1;
  gdouble  t = -1.0, min = 0.0;
  gint     type = -1;
  common  *obj, *bobj = NULL;
  gint     hits = 0;
  GimpVector4   p;

  if ((level == 0) || (imp < 0.005))
    {
      vset (col, 0, 1, 0);
      return 0;
    }

  for (i = 0; i < world.numobj; i++)
    {
      obj = (common *) & world.obj[i];
      switch (obj->type)
	{
	case TRIANGLE:
	  t = checktri (r, (triangle *) & world.obj[i]);
	  break;
	case DISC:
	  t = checkdisc (r, (disc *) & world.obj[i]);
	  break;
	case PLANE:
	  t = checkplane (r, (plane *) & world.obj[i]);
	  break;
	case SPHERE:
	  t = checksphere (r, (sphere *) & world.obj[i]);
	  break;
	case CYLINDER:
	  t = checkcylinder (r, (cylinder *) & world.obj[i]);
	  break;
	default:
	  fprintf (stderr, "Illegal object!!\n");
	  exit (0);
	}
      if (t <= 0.0)
	continue;

      if (!(obj->flags & NOSHADOW) && (level == -1))
	{
	  return i + 1;
	}

      hits++;
      if ((!bobj) || (t < min))
	{

	  min = t;
	  b = i;
	  type = obj->type;
	  bobj = obj;
	}
    }
  if (level == -1)
    return 0;

  if (bobj)
    {
      p.x = r->v1.x + (r->v2.x - r->v1.x) * min;
      p.y = r->v1.y + (r->v2.y - r->v1.y) * min;
      p.z = r->v1.z + (r->v2.z - r->v1.z) * min;

      calclight (col, &p, bobj);

      if (world.flags & SMARTAMBIENT)
	{
	  gdouble ambient = 0.3 * exp (-min / world.smartambient);
	  GimpVector4 lcol;
	  objcolor (&lcol, &p, bobj);
	  vmul (&lcol, ambient);
	  vadd (col, &lcol);
	}

      for (i = 0; i < bobj->numtexture; i++)
	{

	  if ((world.quality >= 4)
	      && ((bobj->texture[i].type == REFLECTION)
		  || (bobj->texture[i].type == PHONG)))
	    {

	      GimpVector4 refcol, norm, ocol;
	      ray ref;

	      objcolor (&ocol, &p, bobj);

	      vcopy (&ref.v1, &p);
	      vcopy (&ref.v2, &r->v1);
	      ref.inside = r->inside;
	      ref.ior = r->ior;

	      vmix (&ref.v1, &ref.v1, &ref.v2, 0.9999);	/* push it a tad */

	      vsub (&ref.v2, &p);
	      objnormal (&norm, bobj, &p);
	      vnorm (&norm, 1.0);
	      vrotate (&norm, 180.0, &ref.v2);

	      vmul (&norm, -0.0001);	/* push it a tad */
	      vadd (&ref.v1, &norm);

	      vnorm (&ref.v2, 1.0);
	      vadd (&ref.v2, &p);

	      if ((world.quality >= 5)
		  && (bobj->texture[i].type == REFLECTION))
		{
		  traceray (&ref, &refcol, level - 1,
			    imp * vmax (&bobj->texture[i].reflection));
		  vvmul (&refcol, &bobj->texture[i].reflection);
		  refcol.w = ocol.w;
		  vadd (col, &refcol);
		}
	      if (bobj->texture[i].type == PHONG)
		{
		  vcset (&refcol, 0, 0, 0, 0);
		  calcphong (bobj, &ref, &refcol);
		  refcol.w = ocol.w;
		  vadd (col, &refcol);
		}

	    }

	  if ((world.quality >= 5) && (col->w < 1.0))
	    {
	      GimpVector4 refcol;
	      ray ref;

	      vcopy (&ref.v1, &p);
	      vcopy (&ref.v2, &p);
	      vsub (&ref.v2, &r->v1);
	      vnorm (&ref.v2, 1.0);
	      vadd (&ref.v2, &p);

	      vmix (&ref.v1, &ref.v1, &ref.v2, 0.999);	/* push it a tad */
	      traceray (&ref, &refcol, level - 1, imp * (1.0 - col->w));
	      vmul (&refcol, (1.0 - col->w));
	      vadd (col, &refcol);
	    }

	  if ((world.quality >= 5) && (bobj->texture[i].type == TRANSPARENT))
	    {
	      GimpVector4 refcol;
	      ray ref;

	      vcopy (&ref.v1, &p);
	      vcopy (&ref.v2, &p);
	      vsub (&ref.v2, &r->v1);
	      vnorm (&ref.v2, 1.0);
	      vadd (&ref.v2, &p);

	      vmix (&ref.v1, &ref.v1, &ref.v2, 0.999);	/* push it a tad */

	      traceray (&ref, &refcol, level - 1,
			imp * vmax (&bobj->texture[i].transparent));
	      vvmul (&refcol, &bobj->texture[i].transparent);

	      vadd (col, &refcol);
	    }

	  if ((world.quality >= 5) && (bobj->texture[i].type == SMOKE))
	    {
	      GimpVector4 smcol, raydir, norm;
	      double tran;
	      ray ref;

	      vcopy (&ref.v1, &p);
	      vcopy (&ref.v2, &p);
	      vsub (&ref.v2, &r->v1);
	      vnorm (&ref.v2, 1.0);
	      vadd (&ref.v2, &p);

	      objnormal (&norm, bobj, &p);
	      vcopy (&raydir, &r->v2);
	      vsub (&raydir, &r->v1);
	      vnorm (&raydir, 1.0);
	      tran = vdot (&norm, &raydir);
	      if (tran < 0.0)
		continue;
	      tran *= tran;
	      vcopy (&smcol, &bobj->texture[i].color1);
	      vmul (&smcol, tran);
	      vadd (col, &smcol);
	    }

	  if ((world.quality >= 5) && (bobj->texture[i].type == REFRACTION))
	    {
	      GimpVector4 refcol, norm, tmpv;
	      ray ref;
	      double c1, c2, n1, n2, n;

	      vcopy (&ref.v1, &p);
	      vcopy (&ref.v2, &p);
	      vsub (&ref.v2, &r->v1);
	      vadd (&ref.v2, &r->v2);

	      vmix (&ref.v1, &ref.v1, &ref.v2, 0.999);	/* push it a tad */

	      vsub (&ref.v2, &p);
	      objnormal (&norm, bobj, &p);

	      if (r->inside == b)
		{
		  ref.inside = -1;
		  ref.ior = 1.0;
		}
	      else
		{
		  ref.inside = b;
		  ref.ior = bobj->texture[i].ior;
		}

	      c1 = vdot (&norm, &ref.v2);

	      if (ref.inside < 0)
		c1 = -c1;

	      n1 = r->ior;	/* IOR of current media  */
	      n2 = ref.ior;	/* IOR of new media  */
	      n = n1 / n2;
	      c2 = 1.0 - n * n * (1.0 - c1 * c1);

	      if (c2 < 0.0)
		{
		  /* FIXME: Internal reflection should occur */
		  c2 = sqrt (-c2);

		}
	      else
		{
		  c2 = sqrt (c2);
		}

	      vmul (&ref.v2, n);
	      vcopy (&tmpv, &norm);
	      vmul (&tmpv, n * c1 - c2);
	      vadd (&ref.v2, &tmpv);

	      vnorm (&ref.v2, 1.0);
	      vadd (&ref.v2, &p);

	      traceray (&ref, &refcol, level - 1,
			imp * vmax (&bobj->texture[i].refraction));

	      vvmul (&refcol, &bobj->texture[i].refraction);
	      vadd (col, &refcol);
	    }
	}
    }
  else
    {
      vcset (col, 0, 0, 0, 0);
      min = 10000.0;
      vcset (&p, 0, 0, 0, 0);
    }

  for (i = 0; i < world.numatmos; i++)
    {
      GimpVector4 tmpcol;
      if (world.atmos[i].type == FOG)
	{
	  gdouble v, pt[3];
	  pt[0] = p.x;
	  pt[1] = p.y;
	  pt[2] = p.z;
	  if ((v = world.atmos[i].turbulence) > 0.0)
	    v = turbulence (pt, 1, 256) * world.atmos[i].turbulence;
	  v = exp (-(min + v) / world.atmos[i].density);
	  vmul (col, v);
	  vcopy (&tmpcol, &world.atmos[i].color);
	  vmul (&tmpcol, 1.0 - v);
	  vadd (col, &tmpcol);
	}
    }

  return hits;
}

static void
setdefaults (texture * t)
{
  memset (t, 0, sizeof (texture));
  t->type = SOLID;
  vcset (&t->color1, 1, 1, 1, 1);
  vcset (&t->color2, 0, 0, 0, 1);
  vcset (&t->diffuse, 1, 1, 1, 1);
  vcset (&t->ambient, 0, 0, 0, 1);
  vset (&t->scale, 1, 1, 1);
  vset (&t->rotate, 0, 0, 0);
  vset (&t->translate, 0, 0, 0);
  t->oscale = 1.0;
  t->amount = 1.0;
  t->exp = 1.0;
}

static gchar *
mklabel (texture * t)
{
  struct textures_t *l;
  static gchar tmps[100];

  if (t->majtype == 0)
    strcpy (tmps, _("Texture"));
  else if (t->majtype == 1)
    strcpy (tmps, _("Bumpmap"));
  else if (t->majtype == 2)
    strcpy (tmps, _("Light"));
  else
    strcpy (tmps, "<unknown>");
  if ((t->majtype == 0) || (t->majtype == 1))
    {
      strcat (tmps, " / ");
      l = textures;
      while (l->s)
	{
	  if (t->type == l->n)
	    {
	      strcat (tmps, gettext (l->s));
	      break;
	    }
	  l++;
	}
    }
  return tmps;
}

static texture *
currenttexture (void)
{
  GtkTreeSelection *sel;
  GtkTreeIter       iter;
  texture          *t = NULL;

  sel = gtk_tree_view_get_selection (texturelist);

  if (gtk_tree_selection_get_selected (sel, NULL, &iter))
    {
      gtk_tree_model_get (gtk_tree_view_get_model (texturelist), &iter,
                          TEXTURE, &t,
                          -1);
    }

  return t;
}

static void
relabel (void)
{
  GtkTreeModel     *model;
  GtkTreeSelection *sel;
  GtkTreeIter       iter;
  texture          *t = NULL;

  sel = gtk_tree_view_get_selection (texturelist);

  if (gtk_tree_selection_get_selected (sel, NULL, &iter))
    {
      model = gtk_tree_view_get_model (texturelist);

      gtk_tree_model_get (model, &iter,
                          TEXTURE, &t,
                          -1);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          TYPE, mklabel (t),
                          -1);
    }
}

static gboolean noupdate = FALSE;

static void
setvals (texture * t)
{
  struct textures_t *l;

  if (!t)
    return;

  noupdate = TRUE;
  gtk_adjustment_set_value (GTK_ADJUSTMENT (amountscale), t->amount);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (scalescale), t->oscale);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (scalexscale), t->scale.x);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (scaleyscale), t->scale.y);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (scalezscale), t->scale.z);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (rotxscale), t->rotate.x);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (rotyscale), t->rotate.y);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (rotzscale), t->rotate.z);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (posxscale), t->translate.x);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (posyscale), t->translate.y);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (poszscale), t->translate.z);

  gtk_adjustment_set_value (GTK_ADJUSTMENT (turbulencescale),
			    t->turbulence.x);
  gtk_adjustment_set_value (GTK_ADJUSTMENT (expscale), t->exp);

  drawcolor1 (NULL);
  drawcolor2 (NULL);

  l = textures;
  while (l->s)
    {
      if (l->n == t->type)
	{
          gimp_int_combo_box_set_active (GIMP_INT_COMBO_BOX (texturemenu),
                                         l->index);
	  break;
	}
      l++;
    }

  gimp_int_combo_box_set_active (GIMP_INT_COMBO_BOX (typemenu), t->majtype);

  noupdate = FALSE;
}

static void
selectitem (GtkTreeSelection *treeselection,
            gpointer          data)
{
  setvals (currenttexture ());
}

static void
addtexture (void)
{
  GtkListStore *list_store;
  GtkTreeIter   iter;
  gint          n = s.com.numtexture;

  if (n == MAXTEXTUREPEROBJ - 1)
    return;

  setdefaults (&s.com.texture[n]);

  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (texturelist));

  gtk_list_store_append (list_store, &iter);
  gtk_list_store_set (list_store, &iter,
                      TYPE,    mklabel (&s.com.texture[n]),
                      TEXTURE, &s.com.texture[n],
                      -1);
  gtk_tree_selection_select_iter (gtk_tree_view_get_selection (texturelist),
                                  &iter);

  s.com.numtexture++;
  restartrender ();
}

static void
duptexture (void)
{
  GtkListStore *list_store;
  GtkTreeIter   iter;
  texture      *t = currenttexture ();
  gint          n = s.com.numtexture;

  if (n == MAXTEXTUREPEROBJ - 1)
    return;
  if (!t)
    return;

  s.com.texture[n] = *t;

  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (texturelist));

  gtk_list_store_append (list_store, &iter);
  gtk_list_store_set (list_store, &iter,
                      TYPE,    mklabel (&s.com.texture[n]),
                      TEXTURE, &s.com.texture[n],
                      -1);
  gtk_tree_selection_select_iter (gtk_tree_view_get_selection (texturelist),
                                  &iter);

  s.com.numtexture++;
  restartrender ();
}

static void
rebuildlist (void)
{
  GtkListStore *list_store;
  GtkTreeIter   iter;
  gint          n;

  for (n = 0; n < s.com.numtexture; n++)
    {
      if (s.com.numtexture && (s.com.texture[n].majtype < 0))
	{
	  gint i;

	  for (i = n; i < s.com.numtexture - 1; i++)
	    s.com.texture[i] = s.com.texture[i + 1];

	  s.com.numtexture--;
	  n--;
	}
    }

  list_store = GTK_LIST_STORE (gtk_tree_view_get_model (texturelist));

  for (n = 0; n < s.com.numtexture; n++)
    {
      gtk_list_store_append (list_store, &iter);
      gtk_list_store_set (list_store, &iter,
                          TYPE,    mklabel (&s.com.texture[n]),
                          TEXTURE, &s.com.texture[n],
                          -1);
    }
  restartrender ();
}

static void
deltexture (void)
{
  GtkTreeSelection *sel;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  texture          *t = NULL;

  sel = gtk_tree_view_get_selection (texturelist);

  if (gtk_tree_selection_get_selected (sel, NULL, &iter))
    {
      model = gtk_tree_view_get_model (texturelist);

      gtk_tree_model_get (model, &iter,
                          TEXTURE, &t,
                          -1);
      t->majtype = -1;
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }
}

static void
loadit (const gchar * fn)
{
  FILE    *f;
  gchar   *end;
  gchar    line[1024];
  gint     i;
  texture *t;

  s.com.numtexture = 0;

  f = fopen (fn, "rt");
  if (!f)
    {
      g_message (_("Could not open '%s' for reading: %s"),
                 gimp_filename_to_utf8 (fn), g_strerror (errno));
      return;
    }

  while (!feof (f))
    {

      if (!fgets (line, 1023, f))
	break;

      i = s.com.numtexture;
      t = &s.com.texture[i];
      setdefaults (t);

      if (sscanf (line, "%d %d %s", &t->majtype, &t->type, end) != 3)
	t->color1.x = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color1.y = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color1.z = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color1.w = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color2.x = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color2.y = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color2.z = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->color2.w = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->oscale = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->turbulence.x = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->amount = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->exp = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->scale.x = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->scale.y = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->scale.z = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->rotate.x = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->rotate.y = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->rotate.z = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->translate.x = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->translate.y = g_ascii_strtod (end, &end);
      if (end && errno != ERANGE)
	t->translate.z = g_ascii_strtod (end, &end);

      s.com.numtexture++;
    }

  fclose (f);
}

static void
loadpreset_response (GtkFileSelection *fs,
                     gint              response_id,
                     gpointer          data)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      const gchar *fn = gtk_file_selection_get_filename (fs);

      gtk_list_store_clear (GTK_LIST_STORE (gtk_tree_view_get_model (texturelist)));
      loadit (fn);
      rebuildlist ();
      restartrender ();
    }

  gtk_widget_hide (GTK_WIDGET (fs));
}

static void
saveit (const gchar *fn)
{
  gint   i;
  FILE  *f;
  gchar  buf[G_ASCII_DTOSTR_BUF_SIZE];

  f = fopen (fn, "wt");
  if (!f)
    {
      g_message (_("Could not open '%s' for writing: %s"),
                 gimp_filename_to_utf8 (fn), g_strerror (errno));
      return;
    }

  for (i = 0; i < s.com.numtexture; i++)
    {
      texture *t = &s.com.texture[i];

      if (t->majtype < 0)
	continue;

      fprintf (f, "%d %d", t->majtype, t->type);
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color1.x));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color1.y));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color1.z));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color1.w));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color2.x));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color2.y));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color2.z));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->color2.w));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->oscale));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->turbulence.x));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->amount));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->exp));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->scale.x));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->scale.y));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->scale.z));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->rotate.x));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->rotate.y));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->rotate.z));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->translate.x));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->translate.y));
      fprintf (f, " %s", g_ascii_dtostr (buf, sizeof (buf), t->translate.z));
      fprintf (f, "\n");
    }

  fclose (f);
}

static void
savepreset_response (GtkFileSelection *fs,
                     gint              response_id,
                     gpointer          data)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      const char *fn = gtk_file_selection_get_filename (fs);

      saveit (fn);
    }

  gtk_widget_hide (GTK_WIDGET (fs));
}

static void
loadpreset (GtkWidget *widget,
            GtkWidget *parent)
{
  fileselect (0, parent);
}

static void
savepreset (GtkWidget *widget,
            GtkWidget *parent)
{
  fileselect (1, parent);
}

static void
fileselect (gint       action,
            GtkWidget *parent)
{
  static GtkWidget *windows[2] = { NULL, NULL };

  gchar *titles[]   = { N_("Open File"), N_("Save File") };
  void  *handlers[] = { loadpreset_response,   savepreset_response };

  if (!windows[action])
    {
      windows[action] = gtk_file_selection_new (gettext (titles[action]));

      gtk_window_set_transient_for (GTK_WINDOW (windows[action]),
                                    GTK_WINDOW (parent));

      gimp_help_connect (windows[action], gimp_standard_help_func,
			 "plug-in-spheredesigner", NULL);

      g_signal_connect (windows[action], "destroy",
			G_CALLBACK (gtk_widget_destroyed),
                        &windows[action]);
      g_signal_connect (windows[action], "delete_event",
			G_CALLBACK (gtk_true),
                        NULL);
      g_signal_connect (windows[action], "response",
			G_CALLBACK (handlers[action]),
                        NULL);
    }

  gtk_window_present (GTK_WINDOW (windows[action]));
}

static void
initworld (void)
{
  gint i;

  memset (&world, 0, sizeof (world));

  s.com.type = SPHERE;
  s.a.x = s.a.y = s.a.z = 0.0;
  s.r = 4.0;

  /* not: world.obj[0] = s;
   * s is a sphere so error C2115: '=' : incompatible types
   */
  memcpy (&world.obj[0], &s, sizeof (s));
  world.numobj = 1;

  world.obj[0].com.numtexture = 0;
  world.obj[0].com.numnormal = 0;

  for (i = 0; i < s.com.numtexture; i++)
    {
      common *c = &s.com;
      common *d = &world.obj[0].com;
      texture *t = &c->texture[i];
      if ((t->amount <= 0.0) || (t->majtype < 0))
	continue;
      if (t->majtype == 0)
	{			/* Normal texture */
	  if (t->type == PHONG)
	    {
	      t->phongcolor = t->color1;
	      t->phongsize = t->oscale / 25.0;
	    }
	  d->texture[d->numtexture] = *t;
	  vmul (&d->texture[d->numtexture].scale,
		d->texture[d->numtexture].oscale);
	  d->numtexture++;
	}
      else if (t->majtype == 1)
	{			/* Bumpmap */
	  d->normal[d->numnormal] = *t;
	  vmul (&d->normal[d->numnormal].scale,
		d->texture[d->numnormal].oscale);
	  d->numnormal++;
	}
      else if (t->majtype == 2)
	{			/* Lightsource */
	  light l;
	  vcopy (&l.a, &t->translate);
	  vcopy (&l.color, &t->color1);
	  vmul (&l.color, t->amount);
	  world.light[world.numlight] = l;
	  world.numlight++;
	}
    }

  world.quality = 5;

  world.flags |= SMARTAMBIENT;
  world.smartambient = 40.0;
}

static void
drawit (void)
{
  if (drawarea)
    gtk_widget_queue_draw (drawarea);
}

static gboolean
expose_event (GtkWidget      *widget,
	      GdkEventExpose *event)
{
  guchar *data = img + event->area.y * 3 * PREVIEWSIZE + event->area.x * 3;

  gdk_draw_rgb_image_dithalign (widget->window,
				widget->style->white_gc,
				event->area.x, event->area.y,
				event->area.width, event->area.height,
				GDK_RGB_DITHER_MAX,
				data, PREVIEWSIZE * 3,
				- event->area.x, - event->area.y);

  return TRUE;
}

static void
restartrender (void)
{
  if (running)
    {
      running = 2;
      return;
    }

  render ();
}

static void
selecttexture (GtkWidget *widget,
               gpointer   data)
{
  texture *t;

  if (noupdate)
    return;

  t = currenttexture ();
  if (!t)
    return;

  gimp_int_combo_box_get_active (GIMP_INT_COMBO_BOX (widget), &t->type);

  relabel ();
  restartrender ();
}

static void
selecttype (GtkWidget *widget,
            gpointer   data)
{
  texture *t;

  if (noupdate)
    return;

  t = currenttexture ();
  if (!t)
    return;

  gimp_int_combo_box_get_active (GIMP_INT_COMBO_BOX (widget), &t->majtype);

  relabel ();
  restartrender ();
}

static void
getscales (GtkWidget *widget,
           gpointer   data)
{
  gdouble f;
  texture *t;

  if (noupdate)
    return;
  t = currenttexture ();
  if (!t)
    return;
  t->amount = GTK_ADJUSTMENT (amountscale)->value;
  t->exp = GTK_ADJUSTMENT (expscale)->value;
  f = GTK_ADJUSTMENT (turbulencescale)->value;
  vset (&t->turbulence, f, f, f);

  t->oscale = GTK_ADJUSTMENT (scalescale)->value;
  t->scale.x = GTK_ADJUSTMENT (scalexscale)->value;
  t->scale.y = GTK_ADJUSTMENT (scaleyscale)->value;
  t->scale.z = GTK_ADJUSTMENT (scalezscale)->value;

  t->rotate.x = GTK_ADJUSTMENT (rotxscale)->value;
  t->rotate.y = GTK_ADJUSTMENT (rotyscale)->value;
  t->rotate.z = GTK_ADJUSTMENT (rotzscale)->value;

  t->translate.x = GTK_ADJUSTMENT (posxscale)->value;
  t->translate.y = GTK_ADJUSTMENT (posyscale)->value;
  t->translate.z = GTK_ADJUSTMENT (poszscale)->value;
}


static void
color1_changed (GimpColorButton *button,
		gpointer         data)
{
  texture *t = currenttexture ();
  if (t)
    {
      gimp_color_button_get_color (button, (GimpRGB *) &t->color1);
    }
}

static void
color2_changed (GimpColorButton *button,
		gpointer         data)
{
  texture *t = currenttexture ();
  if (t)
    {
      gimp_color_button_get_color (button, (GimpRGB *) &t->color2);
    }
}

static void
drawcolor1 (GtkWidget *w)
{
  static GtkWidget *lastw = NULL;

  texture *t = currenttexture ();

  if (w)
    lastw = w;
  else
    w = lastw;
  if (!w)
    return;
  if (!t)
    return;

  gimp_color_button_set_color (GIMP_COLOR_BUTTON (w),
			       (const GimpRGB *) &t->color1);
}

static void
drawcolor2 (GtkWidget *w)
{
  static GtkWidget *lastw = NULL;

  texture *t = currenttexture ();

  if (w)
    lastw = w;
  else
    w = lastw;
  if (!w)
    return;
  if (!t)
    return;

  gimp_color_button_set_color (GIMP_COLOR_BUTTON (w),
			       (const GimpRGB *) &t->color2);
}

static gboolean do_run = FALSE;

static void
sphere_response (GtkWidget *widget,
                 gint       response_id,
                 gpointer   data)
{
  switch (response_id)
    {
    case RESPONSE_RESET:
      s.com.numtexture = 3;

      setdefaults (&s.com.texture[0]);
      setdefaults (&s.com.texture[1]);
      setdefaults (&s.com.texture[2]);

      s.com.texture[1].majtype = 2;
      vset (&s.com.texture[1].color1, 1, 1, 1);
      vset (&s.com.texture[1].translate, -15, -15, -15);

      s.com.texture[2].majtype = 2;
      vset (&s.com.texture[2].color1, 0, 0.4, 0.4);
      vset (&s.com.texture[2].translate, 15, 15, -15);

      gtk_list_store_clear (GTK_LIST_STORE (gtk_tree_view_get_model (texturelist)));
      restartrender ();
      break;

    case GTK_RESPONSE_OK:
      running = -1;
      do_run = TRUE;

    default:
      gtk_widget_hide (widget);
      gtk_main_quit ();
      break;
    }
}

GtkWidget *
makewindow (void)
{
  GtkListStore      *list_store;
  GtkTreeViewColumn *col;
  GtkWidget  *window;
  GtkWidget  *table;
  GtkWidget  *frame;
  GtkWidget  *scrolled;
  GtkWidget  *hbox;
  GtkWidget  *button;
  GtkWidget  *label;
  GtkWidget  *list;
  GtkWidget  *_scalescale;
  GtkWidget  *_rotscale;
  GtkWidget  *_turbulencescale;
  GtkWidget  *_amountscale;
  GtkWidget  *_expscale;
  GimpRGB     rgb;

  window = gimp_dialog_new (_("Sphere Designer"), "spheredesigner",
                            NULL, 0,
			    gimp_standard_help_func,
			    "plug-in-spheredesigner",

                            GIMP_STOCK_RESET, RESPONSE_RESET,
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  g_signal_connect (window, "response",
                    G_CALLBACK (sphere_response),
                    NULL);

  table = gtk_table_new (3, 3, FALSE);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), table);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  gtk_widget_show (table);

  frame = gtk_frame_new (_("Preview"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1,
		    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (frame);

  drawarea = gtk_drawing_area_new ();
  gtk_container_add (GTK_CONTAINER (frame), drawarea);
  gtk_widget_set_size_request (drawarea, PREVIEWSIZE, PREVIEWSIZE);
  gtk_widget_show (drawarea);

  g_signal_connect (drawarea, "expose_event",
		    G_CALLBACK (expose_event), NULL);

  button = gtk_button_new_with_label (_("Update"));
  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
		    G_CALLBACK (restartrender), NULL);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
				       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_AUTOMATIC);
  gtk_table_attach (GTK_TABLE (table), scrolled, 1, 2, 0, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_widget_show (scrolled);

  list_store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
  list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);

  texturelist = GTK_TREE_VIEW (list);

  g_signal_connect (gtk_tree_view_get_selection (texturelist), "changed",
		    G_CALLBACK (selectitem),
                    NULL);

  gtk_widget_set_size_request (list, -1, 150);
  gtk_container_add (GTK_CONTAINER (scrolled), list);
  gtk_widget_show (list);

  col = gtk_tree_view_column_new_with_attributes (_("Textures"),
                                                  gtk_cell_renderer_text_new (),
                                                  "text", TYPE,
                                                  NULL);
  gtk_tree_view_append_column (texturelist, col);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_from_stock (GTK_STOCK_NEW);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect_swapped (button, "clicked",
			    G_CALLBACK (addtexture), NULL);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GIMP_STOCK_DUPLICATE);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect_swapped (button, "clicked",
			    G_CALLBACK (duptexture), NULL);
  gtk_widget_show (button);

  button = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  g_signal_connect_swapped (button, "clicked",
			    G_CALLBACK (deltexture), NULL);
  gtk_widget_show (button);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (loadpreset),
                    window);

  button = gtk_button_new_from_stock (GTK_STOCK_SAVE);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (savepreset),
                    window);

  frame = gtk_frame_new (_("Texture Properties"));
  gtk_table_attach (GTK_TABLE (table), frame, 2, 3, 0, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (6, 4, FALSE);
  gtk_widget_show (table);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);

  label = gtk_label_new (_("Type:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_widget_show (label);

  label = gtk_label_new (_("Texture:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);

  label = gtk_label_new (_("Colors:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_widget_show (hbox);

  button = gimp_color_button_new (_("Color Selection Dialog"),
				  COLORBUTTONWIDTH, COLORBUTTONHEIGHT, &rgb,
				  GIMP_COLOR_AREA_FLAT);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
  gtk_widget_show (button);
  drawcolor1 (button);

  g_signal_connect (button, "color_changed",
		    G_CALLBACK (color1_changed),
		    NULL);

  button = gimp_color_button_new (_("Color Selection Dialog"),
				  COLORBUTTONWIDTH, COLORBUTTONHEIGHT, &rgb,
				  GIMP_COLOR_AREA_FLAT);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
  gtk_widget_show (button);
  drawcolor2 (button);

  g_signal_connect (button, "color_changed",
		    G_CALLBACK (color2_changed),
		    NULL);

  label = gtk_label_new (_("Scale:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (scalescale =
		     gtk_adjustment_new (1.0, 0.0, 5.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 1, 2, 3, 4,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  g_signal_connect (scalescale, "value_changed",
		    G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Turbulence:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _turbulencescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (turbulencescale =
		     gtk_adjustment_new (0.0, 0.0, 5.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_turbulencescale, 100, -1);
  gtk_widget_show (_turbulencescale);
  gtk_table_attach (GTK_TABLE (table), _turbulencescale, 1, 2, 4, 5,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  gtk_scale_set_digits (GTK_SCALE (_turbulencescale), 2);
  g_signal_connect (turbulencescale, "value_changed",
		    G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Scale X:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (scalexscale =
		     gtk_adjustment_new (1.0, 0.0, 5.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 3, 4, 0, 1,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (scalexscale, "value_changed",
		    G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Scale Y:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (scaleyscale =
		     gtk_adjustment_new (1.0, 0.0, 5.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 3, 4, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (scaleyscale, "value_changed",
		    G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Scale Z:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (scalezscale =
		     gtk_adjustment_new (1.0, 0.0, 5.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 3, 4, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (scalezscale, "value_changed",
		    G_CALLBACK (getscales), NULL);


  label = gtk_label_new (_("Rotate X:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 3, 4,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _rotscale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (rotxscale =
		     gtk_adjustment_new (1.0, 0.0, 360.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_rotscale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_rotscale), 2);
  gtk_widget_show (_rotscale);
  gtk_table_attach (GTK_TABLE (table), _rotscale, 3, 4, 3, 4,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (rotxscale, "value_changed", G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Rotate Y:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 4, 5,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _rotscale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (rotyscale =
		     gtk_adjustment_new (1.0, 0.0, 360.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_rotscale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_rotscale), 2);
  gtk_widget_show (_rotscale);
  gtk_table_attach (GTK_TABLE (table), _rotscale, 3, 4, 4, 5,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (rotyscale, "value_changed", G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Rotate Z:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 5, 6,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _rotscale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (rotzscale =
		     gtk_adjustment_new (1.0, 0.0, 360.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_rotscale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_rotscale), 2);
  gtk_widget_show (_rotscale);
  gtk_table_attach (GTK_TABLE (table), _rotscale, 3, 4, 5, 6,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (rotzscale, "value_changed", G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Pos X:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 5, 6, 0, 1,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (posxscale =
		     gtk_adjustment_new (0.0, -20.0, 20.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 6, 7, 0, 1,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (posxscale, "value_changed", G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Pos Y:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 5, 6, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (posyscale =
		     gtk_adjustment_new (1.0, -20.0, 20.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 6, 7, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (posyscale, "value_changed", G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Pos Z:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 5, 6, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _scalescale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (poszscale =
		     gtk_adjustment_new (1.0, -20.0, 20.1, 0.1, 0.1, 0.1)));
  gtk_widget_set_size_request (_scalescale, 100, -1);
  gtk_scale_set_digits (GTK_SCALE (_scalescale), 2);
  gtk_widget_show (_scalescale);
  gtk_table_attach (GTK_TABLE (table), _scalescale, 6, 7, 2, 3,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  g_signal_connect (poszscale, "value_changed", G_CALLBACK (getscales), NULL);

  typemenu = gimp_int_combo_box_new (_("Texture"), 0,
                                     _("Bump"),    1,
                                     _("Light"),   2,
                                     NULL);
  gimp_int_combo_box_connect (GIMP_INT_COMBO_BOX (typemenu), 0,
                              G_CALLBACK (selecttype),
                              NULL);

  gtk_table_attach (GTK_TABLE (table), typemenu, 1, 2, 0, 1,
		    GTK_FILL | GTK_EXPAND, GTK_EXPAND, 0, 0);
  gtk_widget_show (typemenu);

  texturemenu = gimp_int_combo_box_new (NULL, 0);

  {
    struct textures_t *t;

    for (t = textures; t->s; t++)
      gimp_int_combo_box_append (GIMP_INT_COMBO_BOX (texturemenu),
                                 GIMP_INT_STORE_VALUE, t->n,
                                 GIMP_INT_STORE_LABEL, gettext (t->s),
                                 -1);
  }

  gimp_int_combo_box_connect (GIMP_INT_COMBO_BOX (texturemenu), 0,
                              G_CALLBACK (selecttexture),
                              NULL);

  gtk_table_attach (GTK_TABLE (table), texturemenu, 1, 2, 1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND, 0, 0);
  gtk_widget_show (texturemenu);

  label = gtk_label_new (_("Amount:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _amountscale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (amountscale =
		     gtk_adjustment_new (1.0, 0, 1.01, .01, .01, .01)));
  gtk_widget_set_size_request (_amountscale, 100, -1);
  gtk_widget_show (_amountscale);
  gtk_table_attach (GTK_TABLE (table), _amountscale, 1, 2, 5, 6,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_scale_set_digits (GTK_SCALE (_amountscale), 2);
  g_signal_connect (amountscale, "value_changed",
		    G_CALLBACK (getscales), NULL);

  label = gtk_label_new (_("Exp:"));
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);

  _expscale =
    gtk_hscale_new (GTK_ADJUSTMENT
		    (expscale =
		     gtk_adjustment_new (1.0, 0, 1.01, .01, .01, .01)));
  gtk_widget_set_size_request (_expscale, 100, -1);
  gtk_widget_show (_expscale);
  gtk_table_attach (GTK_TABLE (table), _expscale, 1, 2, 6, 7,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_scale_set_digits (GTK_SCALE (_expscale), 2);
  g_signal_connect (expscale, "value_changed", G_CALLBACK (getscales), NULL);

  gtk_widget_show (window);

  return window;
}

static guchar
pixelval (gdouble v)
{
  v += 0.5;
  if (v < 0.0)
    return 0;
  if (v > 255.0)
    return 255;
  return v;
}

static void
render (void)
{
  gint    x, y, p;
  ray     r;
  GimpVector4  col;
  gint    hit;
  gint    tx, ty;
  guchar *dest_row;
  gint    bpp;

  r.v1.z = -10.0;
  r.v2.z = 0.0;

  running = 2;

  tx = PREVIEWSIZE;
  ty = PREVIEWSIZE;
  bpp = 3;

  while (running > 0)
    {

      if (running == 2)
	{
	  running = 1;
	  initworld ();
	}
      if (world.obj[0].com.numtexture == 0)
	break;

      for (y = 0; y < ty; y++)
	{
	  dest_row = img + y * PREVIEWSIZE * 3;

	  for (x = 0; x < tx; x++)
	    {
	      gint g, gridsize = 16;

	      g = ((x / gridsize + y / gridsize) % 2) * 60 + 100;

	      r.v1.x = r.v2.x = 8.5 * (x / (float) (tx - 1) - 0.5);
	      r.v1.y = r.v2.y = 8.5 * (y / (float) (ty - 1) - 0.5);

	      p = x * bpp;

	      hit = traceray (&r, &col, 10, 1.0);

	      if (col.w < 0.0)
		col.w = 0.0;
	      else if (col.w > 1.0)
		col.w = 1.0;

	      dest_row[p + 0] =
		pixelval (255 * col.x) * col.w + g * (1.0 - col.w);
	      dest_row[p + 1] =
		pixelval (255 * col.y) * col.w + g * (1.0 - col.w);
	      dest_row[p + 2] =
		pixelval (255 * col.z) * col.w + g * (1.0 - col.w);

	      if (running != 1)
		{
		  break;
		}
	    }

#if CONTINOUS_UPDATE
	  drawit ();

	  while (gtk_events_pending ())
	    gtk_main_iteration ();
#endif

	  if (running != 1)
	    {
	      break;
	    }
	}
      if (running == 1)
	break;
      if (running == -1)
	break;
    }

  running = 0;
  drawit ();
}

static void
realrender (GimpDrawable *drawable)
{
  gint          x, y;
  ray           r;
  GimpVector4   rcol;
  gint          tx, ty;
  gint          x1, y1, x2, y2;
  guchar       *dest;
  gint          bpp;
  GimpPixelRgn  pr, dpr;
  guchar       *buffer, *ibuffer;

  if (running > 0)
    return;			/* Fixme: abort preview-render instead! */

  initworld ();

  r.v1.z = -10.0;
  r.v2.z = 0.0;

  gimp_pixel_rgn_init (&pr, drawable, 0, 0,
		       gimp_drawable_width (drawable->drawable_id),
		       gimp_drawable_height (drawable->drawable_id), FALSE,
		       FALSE);
  gimp_pixel_rgn_init (&dpr, drawable, 0, 0,
		       gimp_drawable_width (drawable->drawable_id),
		       gimp_drawable_height (drawable->drawable_id), TRUE,
		       TRUE);
  gimp_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
  bpp = gimp_drawable_bpp (drawable->drawable_id);
  buffer = g_malloc ((x2 - x1) * 4);
  ibuffer = g_malloc ((x2 - x1) * 4);

  tx = x2 - x1;
  ty = y2 - y1;

  gimp_progress_init (_("Rendering Sphere..."));

  for (y = 0; y < ty; y++)
    {
      dest = buffer;
      for (x = 0; x < tx; x++)
	{
	  r.v1.x = r.v2.x = 8.1 * (x / (float) (tx - 1) - 0.5);
	  r.v1.y = r.v2.y = 8.1 * (y / (float) (ty - 1) - 0.5);

	  traceray (&r, &rcol, 10, 1.0);
	  dest[0] = pixelval (255 * rcol.x);
	  dest[1] = pixelval (255 * rcol.y);
	  dest[2] = pixelval (255 * rcol.z);
	  dest[3] = pixelval (255 * rcol.w);
	  dest += 4;
	}
      gimp_pixel_rgn_get_row (&pr, ibuffer, x1, y1 + y, x2 - x1);
      for (x = 0; x < (x2 - x1); x++)
	{
	  gint   k, dx = x * 4, sx = x * bpp;
	  gfloat a     = buffer[dx + 3] / 255.0;

	  for (k = 0; k < bpp; k++)
	    {
	      ibuffer[sx + k] =
		buffer[dx + k] * a + ibuffer[sx + k] * (1.0 - a);
	    }
	}
      gimp_pixel_rgn_set_row (&dpr, ibuffer, x1, y1 + y, x2 - x1);
      gimp_progress_update ((gdouble) y / (gdouble) ty);
    }
  g_free (buffer);
  g_free (ibuffer);
  gimp_drawable_flush (drawable);
  gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
  gimp_drawable_update (drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
}

static void
query (void)
{
  static GimpParamDef args[] =
  {
    { GIMP_PDB_INT32,    "run_mode", "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE,    "image",    "Input image (unused)"         },
    { GIMP_PDB_DRAWABLE, "drawable", "Input drawable"               }
  };

  gimp_install_procedure ("plug_in_spheredesigner",
			  "Renders textures spheres",
			  "This plugin can be used to create textured and/or "
			  "bumpmapped spheres, and uses a small lightweight "
			  "raytracer to perform the task with good quality",
			  "Vidar Madsen",
			  "Vidar Madsen",
			  "1999",
			  N_("<Image>/Filters/Render/Sphere _Designer..."),
			  "RGB*, GRAY*",
			  GIMP_PLUGIN, G_N_ELEMENTS (args), 0, args, NULL);
}

static gboolean
sphere_main (GimpDrawable *drawable)
{
  initworld ();

  gimp_ui_init ("spheredesigner", TRUE);

  memset (img, 0, PREVIEWSIZE * PREVIEWSIZE * 3);
  makewindow ();

  if (!s.com.numtexture)
    sphere_response (NULL, RESPONSE_RESET, 0);

  rebuildlist ();

  gtk_main ();

  return do_run;
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[1];
  GimpDrawable      *drawable;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;

  drawable = gimp_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case GIMP_RUN_INTERACTIVE:
      s.com.numtexture = 0;
      gimp_get_data (PLUG_IN_NAME, &s);
      if (!sphere_main (drawable))
	{
	  gimp_drawable_detach (drawable);
	  return;
	}
      break;
    case GIMP_RUN_WITH_LAST_VALS:
      s.com.numtexture = 0;
      gimp_get_data (PLUG_IN_NAME, &s);
      if (s.com.numtexture == 0)
	{
	  gimp_drawable_detach (drawable);
	  return;
	}
      break;
    case GIMP_RUN_NONINTERACTIVE:
    default:
      /* Not implementet yet... */
      gimp_drawable_detach (drawable);
      return;
    }

  gimp_set_data (PLUG_IN_NAME, &s, sizeof (s));

  realrender (drawable);
  gimp_displays_flush ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = status;

  gimp_drawable_detach (drawable);
}

MAIN ()
