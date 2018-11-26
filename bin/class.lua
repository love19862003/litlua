function test( cc) 
    local t = {
	p = function(str)
		print("function p call print:"..str)
	end
  }
    print("\n\n\n\ncall test function print cc")
	cc:print1()
	cc:set(false)
	cc:print1()
	cc:set_base("dddddddddddddddddddddddddddddddddddddddd")
	return 11, 1,2,3,4,5,t
end

print("gtest", gtest);
gtest = gtest + 1
print("gtest", gtest);
local t = {
	p = function(str)
		print("function p call print:"..str)
	end
}

local test = CTest("")
test:stable(1, t, 3)



print("\n\n\n\ntest:stable() call end && \n\n\nprint table t")
for k, v in pairs(t) do
	if type(v) ~= "function" then
		print(k .. " = " .. v)
	else
		print( "function: " .. k)
	end
end


local base = CBase("this is the base class")
base:print_base()
base:set_base("change base value")
base:print_base()
test:set_base("this is real test Class && then call test:print_base")
test:print_base()

--test()
