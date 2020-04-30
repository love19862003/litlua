printStr("printStr1", "printStr2", "printStr3")
printStr(1,2,3)
printStr(nil, 1, "printStr")

local a = 100 
local b = add(a)
assert(b == a + 1)
printStr(b, " = ", a .." + 1")
assert( b == test(a, b))
print("call func")
print(func(1, 100)) -- this will error
print("call func2")
print(func2(1, 100))
print("call once func3 ")
print(func3())
print("call twice func3")
print(func3())
