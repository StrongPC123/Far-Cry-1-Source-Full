

template examples:
* normalmap_high
* normalmap_low
* skybox


todo:
* reduce resolution with add and sub
* presets
* platform settings (low mem, compressed normalmaps, high quality version, ...)
* apply greyscale bumpmap to normalmap
* D3DXComputeNormalMap
* test reduce resolution bejond 1:1 result
* directory specific parameters
* extract data from the huge polybump output file
* close window = cancel
* non power of two warning
* check for resource compiler (is gfx support enougth)
* Init() with access to log, error and info


future todo:
* gamma, brightness
* support premultiplied alpha (DXT2 and DXT4)
* 8bit, 4bit indexed colors (with or without luminance)
* statistics (frequency)
* ifdef for localization and special versions
* mask color for lightmaps and border colors
* better mipmap filter kernel




----------------------------------------------------------------------------------------------------

e.g.

template=skybox_texture



e.g.

pixelformat=DXT1					; default
mipmaps=none



e.g.

pixelformat=R8G8B8
mipmaps=max							; default
mipmirror=0




e.g.

pixelformat=DXT3
mipmaps=3
mipmirror=1							; default







