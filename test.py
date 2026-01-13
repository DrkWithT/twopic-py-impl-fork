def greet(name):
    message = "Hello, " + name
    print(message)
    return message

def pass1():
    pass

def hello():
    def hello2():
        print("hello")
    print("hello")

c = 1
c += 1

plus = 3 * 2 - 1
plus2 = 1 + 2 * 3

while c:
    print("hello")

def switch_case(value):
    match value:
        case 1:
            return "one"
        case 2:
            return "two"
        
class new_world:
    def __init__(self, world):
        self.world = world
    
wrld = new_world("world")

x = 42
y = 3.14
greet("World")

try:
  print("Hello")
except:
  print("Something went wrong")
else:
    print("else")

try:
  print(x)
except:
  print("Something went wrong")
finally:
  print("The 'try except' is finished")
    