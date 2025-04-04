Wenn ich Kurven als WT exportieren will, muss ich die ja sampeln mit sagen wir 4096 diskreten Punkten. Also kann ich ja auch einfach die kurven aus Segmenten kreiren? Aber dann wird es ungenauer zu zeichnen?
Kann ich nicht auch die finale kuve aus was auch immer exportieren als expression?



## SDL til
### KeyInput
- use `key.repeat` for single keypress
- can fetch `SDL_GetKeyboardState`


if for all i dont care for distance, same for all selected. only for hl klick in segmentation mode

i render by just discrete scanning from left trough right

nimm jedes object, berechne intersections mit allen anderern objekten, sortiere aufsteigend, erstelle segmente von kleinstem paar bis groestem paar, speichere in seglist

-- verschiedene funktion if objekt circle oder line

for every new or deleted object, clear free segments and do it again

segments have objectStatus, draw all status=selected
