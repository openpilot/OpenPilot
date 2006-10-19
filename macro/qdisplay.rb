require 'jafar/kernel'
require 'jafar/qdisplay/qdisplay'
Jafar.register_module Jafar::Qdisplay

module Jafar
module Qdisplay

def Qdisplay.showfile(filename)
viewer = Jafar::Qdisplay::Viewer.new
image = Jafar::Image::Image.loadImage(filename)
imageitem = Jafar::Qdisplay::ImageItem.new(image)
viewer.setImageItem(imageitem)
return viewer
end

end
end
