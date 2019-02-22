import random;
import sys;
for f in range(3):
    string = "";
    for i in range(10):
        string = string + chr(random.randint(97, 122));
    sys.stdout.write(string + "\n");
    open("file"+str(f),'w').write(string + "\n");
a = random.randint(1,42);
b = random.randint(1,42);
sys.stdout.write(str(a)+"\n"+str(b)+"\n"+str(a*b)+"\n");
