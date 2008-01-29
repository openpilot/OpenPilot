require 'jafar/kernel'
require 'jafar/qdisplay/qdisplay'
Jafar.register_module Jafar::Qdisplay

module Jafar
module Qdisplay

if(not constants.include?("VManager"))
  VManager = ViewerManager.new
end

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
    
class DisplayEventHandler < AbstractEventHandler
  def initialize
    super()
  end
  def mouseReleaseEvent(button, x, y)
    puts "Button: #{button} Coordinate: (#{x}, #{y})"
  end
end

class RobotTrajectory < PolyLine
  attr_reader :lastPose
  def initialize(scale = 10.0)
    super(scale)
    require 'jafar/geom'
    require 'jafar/jmath'
    @lastPose = nil
  end
  def moveTo(robotPose)
    arr = Jmath.vecToArray(robotPose.getX)
    addPoint(arr[0] , arr[1] )
    @lastPose = robotPose
  end
  def moveOf(robotMvt)
    if(@lastPose.nil?)
      @lastPose = robotMvt
    else
      nextPose = Geom::T3DEuler.new
      Geom::T3D.compose( @lastPose, robotMvt, nextPose)
      moveTo(nextPose)
    end
  end
end
    
end
end
