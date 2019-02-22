import random;
import sys;
str = "";
for f in range 3:
    for i in range(10):
        str = str + chr(random.randint(97, 122));
    sys.stdout.write(str + "\n");
open("file"+str(f),'w').write(str + "\n");
a = random.randint(1,42);
b = random.randint(1,42);
sys.stdout.write(str(a)+"\n"+str(b)+"\n"+str(a*b)+"\n");
