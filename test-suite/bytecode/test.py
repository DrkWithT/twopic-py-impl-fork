import dis

code = """

def add(x):
    y = x + 1
    return y

y = 0

add(y)

"""

dis.dis(code)