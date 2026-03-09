import dis

code = """

x = 1
y = 2

z = x + y

print(z)
"""

dis.dis(code)