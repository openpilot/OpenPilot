How to use the Dials artwork:

- The file 'dials-master.svg' is the master file, as its name suggests. This means that it is the reference, and that all other SVG files in the directory are extracts from the master.

The master SVG contains layers for all elements: background, rotating/moving elements, and fixed elements.

extracts directory:

  - Dial background is contained in dials-dark-background.svg

  - Then for each dial type, there are several SVG files, one for each element: ELEMENT-00N-R or ELEMENT-00N-F depending on whether the layer should be fixed or rotating, and the order number hinting at the order by which it should be displayed.

   TODO: ideally, extracting the layers from the master SVG should be automated, but for now we are doing it manually.

defaultset directory:

   This directory contains dials which are ready for use in the GCS.