## CPU vs GPU
- any rendering algorithm could be implemented on either CPU or GPU
- 'Graphics Pipeline GPU': transform 3D models into a 2D image
  - 'Fixed-Function' (not programmable) like rasterization, blending, ...
    - can only be configuered not controlled
  - 'Programmable Stages' like vertex shaders, ...
    - one can control this stages with shaders
  - Some stuff is fixed because well defined, efficient, standardized
    - no need to know how gpu implements e.g. rasterization

  - 'Graphics Pibeline CPU': same, but no standard
    - on the CPU all stages have to be implemented and are thus programmable

- graphics on esp32 can only be software rendered, can use smid

## esp32
- uses 'newlib' as C standardlib implementation (nearly everything supported)

