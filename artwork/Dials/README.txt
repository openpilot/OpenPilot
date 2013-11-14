How to use the Dials artwork:

- The file 'default-wip/dials-master.svg' is the master file, as its name suggests. This means that it is the reference, and that all other SVG files in the directory are extracts from the master.

The master SVG contains layers for all elements: background, rotating/moving elements, and fixed elements.

default directory:

   All the dials used in the GCS today, but with all text elements still as "text". This means that depending on whether the target platform knows the font, the dial might render differently. For this reason, the dials which are used by the GCS are located in ground/share/dials/default and all text elements there are converted to "path"
