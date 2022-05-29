from datetime import datetime
#data = str(str(datetime.now()).split(".")).split(" ")
print(datetime.now())
print(str(datetime.now()).split("."))
print(str(datetime.now()).split(".")[0])
print(str(datetime.now()).split(".")[1])
print(str(str(datetime.now()).split(".")[0]).split("'"))