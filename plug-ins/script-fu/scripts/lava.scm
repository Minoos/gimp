; The GIMP -- an image manipulation program
; Copyright (C) 1995 Spencer Kimball and Peter Mattis
; 
; Lava effect
; Copyright (c) 1997 Adrian Likins
; aklikins@eos.ncsu.edu
;
; based on a idea by Sven Riedel <lynx@heim8.tu-clausthal.de>
; tweaked a bit by Sven Neumann <neumanns@uni-duesseldorf.de>
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


(define (script-fu-lava image
			drawable
			seed
			tile_size
			mask_size
			gradient
			keep-selection
			separate-layer
			current-grad)
  (let* (
	 (type (car (gimp-drawable-type-with-alpha drawable)))
	 (image-width (car (gimp-image-width image)))
	 (image-height (car (gimp-image-height image))))
    
    (gimp-context-push)

    (gimp-image-undo-disable image)
    (gimp-layer-add-alpha drawable)
    
    (if (= (car (gimp-selection-is-empty image)) TRUE)
	(begin
	  (gimp-selection-layer-alpha drawable)
	  (set! active-selection (car (gimp-selection-save image)))
	  (set! from-selection FALSE))
	(begin
	  (set! from-selection TRUE)
	  (set! active-selection (car (gimp-selection-save image)))))
    
    (set! selection-bounds (gimp-selection-bounds image))
    (set! select-offset-x (cadr selection-bounds))
    (set! select-offset-y (caddr selection-bounds))
    (set! select-width (- (cadr (cddr selection-bounds)) select-offset-x))
    (set! select-height (- (caddr (cddr selection-bounds)) select-offset-y))
    
    (if (= separate-layer TRUE)
	(begin
	  (set! lava-layer (car (gimp-layer-new image
						select-width
						select-height
						type
						"Lava Layer"
						100
						NORMAL-MODE)))
	  
	  (gimp-image-add-layer image lava-layer -1)
	  (gimp-layer-set-offsets lava-layer select-offset-x select-offset-y)
	  (gimp-selection-none image)
	  (gimp-edit-clear lava-layer)
	  
	  (gimp-selection-load active-selection)
	  (gimp-image-set-active-layer image lava-layer)))
    
    (set! active-layer (car (gimp-image-get-active-layer image)))
    
    (if (= current-grad FALSE)
	(gimp-gradients-set-gradient gradient))
    
    (plug-in-solid-noise 1 image active-layer FALSE TRUE seed 2 2 2)
    (plug-in-cubism 1 image active-layer tile_size 2.5 0)
    (plug-in-oilify 1 image active-layer mask_size 0)
    (plug-in-edge 1 image active-layer 2 0 0)
    (plug-in-gauss-rle 1 image active-layer 2 TRUE TRUE)
    (plug-in-gradmap 1 image active-layer)

    (if (= keep-selection FALSE)
	(gimp-selection-none image))
   
    (gimp-image-set-active-layer image drawable)
    (gimp-image-remove-channel image active-selection)
    (gimp-image-undo-enable image)
    (gimp-displays-flush)

    (gimp-context-pop)))

(script-fu-register "script-fu-lava"
		    _"<Image>/Script-Fu/Render/_Lava..."
		    "Fills the current selection with lava."
		    "Adrian Likins <adrian@gimp.org>"
		    "Adrian Likins"
		    "10/12/97"
		    "RGB* GRAY*"
		    SF-IMAGE "Image" 0
		    SF-DRAWABLE "Drawable" 0
		    SF-ADJUSTMENT _"Seed" '(10 1 30000 1 10 0 1)
		    SF-ADJUSTMENT _"Size" '(10 0 100 1 10 0 1)
		    SF-ADJUSTMENT _"Roughness" '(7 3 50 1 10 0 0)
		    SF-GRADIENT _"Gradient" "German flag smooth"
		    SF-TOGGLE   _"Keep selection" TRUE
		    SF-TOGGLE   _"Separate layer" TRUE
		    SF-TOGGLE   _"Use current gradient" FALSE)
