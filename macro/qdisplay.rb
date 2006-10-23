require 'jafar/kernel'
require 'jafar/qdisplay/qdisplay'
Jafar.register_module Jafar::Qdisplay

module Jafar
module Qdisplay

def Qdisplay.showfile(filename)
viewer = Jafar::Qdisplay::Viewer.new
image = Jafar::Image::Image.loadImage(filename)
imageview = Jafar::Qdisplay::ImageView.new(image)
viewer.setImageView(imageview)
return viewer
end

end
end
