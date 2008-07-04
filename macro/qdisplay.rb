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
  def initialize(scale = 10.0, indexX = 0, indexY = 1)
    super(scale)
    require 'jafar/geom'
    require 'jafar/jmath'
    @lastPose = nil
    @indexX = indexX
    @indexY = indexY
  end
  def moveTo(robotPose)
    arr = Jmath.vecToArray(robotPose.getX)
    addPoint(arr[@indexX] , arr[@indexY] )
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

def Qdisplay.showPoints( image, arrPoints, showCount = false, color = [255,0,0] )
  viewer = Qdisplay::Viewer.new
  view = Qdisplay::ImageView.new( image )
  viewer.setImageView( view )
  count = 0
  arrPoints.each() { |pt|
    shape = Qdisplay::Shape.new(Qdisplay::Shape::ShapeCross, pt[0], pt[1], 1, 1)
    if( showCount )
      shape.setFontSize( 1 )
      shape.setFontColor( color[0], color[1], color[2] )
      shape.setLabel( count.to_s )
    end
    count += 1
    view.addShape( shape )
  }
end

end
end
