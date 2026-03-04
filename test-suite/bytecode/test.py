import dis

code = """
x = 1
y = 2

if x or y:
    print(x)
    print(y) 
else:
    print(x)

z = 1
"""

dis.dis(code)