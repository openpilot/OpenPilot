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

def Qdisplay.showimages(images)
  viewer = Jafar::Qdisplay::Viewer.new
  pos = 0
  images.each() { |image|
  imageview = Jafar::Qdisplay::ImageView.new(image)
    viewer.setImageView(imageview, pos)
    pos += 1
  }
  return viewer
end

def Qdisplay.showfile(filename)
image = Jafar::Image::Image.loadImage(filename)
return Qdisplay.showimage(image)
end
    
#
# EventHandler that you can use with a display object to display
# the X and Y coordinates where the display was clicked
#
class DisplayEventHandler < AbstractEventHandler
  def initialize
    super()
  end
  def mouseReleaseEvent(button, x, y)
    puts "Button: #{button} Coordinate: (#{x}, #{y})"
  end
end
#
# This class allow to draw a RobotTrajectory on a display viewer.
# It's basically a PolyLine with a few convenient function to
# add robot pose or robot movement with a T3DEuler
#
class RobotTrajectory < PolyLine
  attr_reader :lastPose
  #
  # indexX the index of the T3D vector used for the X-coordinate
  # indexY the index of the T3D vector used for the Y-coordinate 
  #
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

#
# Always colorize shape with the same color
#
class ConstantColorizer
  def initialize( color = [255, 0, 0] )
    @color = color
  end
  def colorizeFor( id, shape )
    shape.setColor( @color[0], @color[1], @color[2] )
  end
  def color( id )
    return @color
  end
end

#
# Colorize shapes with a different color for each id
#
class OneColorPerIdColorizer
  def initialize
    @colors = {}
  end
  def colorizeFor( id, shape )
    c = @colors[id]
    if( c.nil? )
      c = [ (rand * 255).to_i, (rand * 255).to_i, (rand * 255).to_i ]
      @colors[id] = c
    end
    shape.setColor( c[0], c[1], c[2] )
  end
  def color( id )
    return @colors[id]
  end
end

#
# Show a list points on an image inside a viewer
#
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

#
# Show the gradients of an image
#

def Qdisplay.showGradients( image )
  # Compute Sobel
  sobelX = Image::Image.new( image.width(), image.height(), Image::IPL_DEPTH_16S, Image::JfrImage_CS_GRAY )
  Image::cvSobel( image, sobelX, 1,0,3)
  sobelY = Image::Image.new( image.width(), image.height(), Image::IPL_DEPTH_16S, Image::JfrImage_CS_GRAY )
  Image::cvSobel( image, sobelY, 0,1,3)
  
  # Display image
  viewer = Qdisplay::Viewer.new
  view = Qdisplay::ImageView.new( image )
  viewer.setImageView( view )
  viewer.setUpdatesEnabled( false )
  # Draw lines
  for y in 0...image.height
    puts "#{y} / #{image.height}"
    for x in 0...image.width
      dy = sobelY.getPixelValueShort( x,y );
      dx = sobelX.getPixelValueShort( x,y );
      norm = Math.sqrt( dx * dx + dy * dy ) / 255
      angle = Math.atan2( dy, dx )
      l = Qdisplay::Line.new( x + 0.5, y + 0.5, x + 0.5 + norm * Math.cos(angle), y + 0.5 + norm * Math.sin( angle ) )
      view.addLine( l )
    end
  end
  viewer.setUpdatesEnabled( true )
end

end
end
