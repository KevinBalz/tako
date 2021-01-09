import "tako" for Tako, Color

System.print("Wren!")
var x = 0

var red = Color.new(255, 0, 0, 255)
var blue = Color.new(0, 0, 255, 255)

Tako.update {|dt|
    x = x + dt * 10
    blue.r = blue.r + 1
    blue.b = blue.b + 2
    blue.g = blue.g + 3
}

System.print("%(blue.r) %(blue.g) %(blue.b) %(blue.a)")

Tako.draw {|drawer|
    drawer.clear()
    drawer.drawRect(50, 50, 50, 50, red)
    drawer.drawRect(x, 0, 16, 16, blue)
}