import "tako" for Tako

System.print("Heyo!")
var x = 0
Tako.draw {
    Tako.drawRect(50, 50, 50, 50)
    Tako.drawRect(x, 0, 16, 16)
    x = x + 1
}