require 'jafar/kernel'
require 'jafar/qdisplay/qdisplay'
Jafar.register_module Jafar::Qdisplay

module Jafar
module Qdisplay

def Qdisplay.showimage(image)
viewer = Jafar::Qdisplay::Viewer.new
imageview = Jafar::Qdisplay::ImageView.new(image)
viewer.setImageView(imageview)
return viewer
end

def Qdisplay.showfile(filename)
image = Jafar::Image::Image.loadImage(filename)
return Qdisplay.showimage(image)
end

end
end
