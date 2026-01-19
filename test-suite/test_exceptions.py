x = 42

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
