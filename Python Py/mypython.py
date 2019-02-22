import random;
import sys;
random.seed();

def generate_string(numChars):
    str = "";
    for i in range(numChars):
        str = str + chr(random.randint(97, 122));
    str = str + "\n";
    sys.stdout.write(str);
    return str;

file1 = open("file1",'w').write(generate_string(10));
file2 = open("file2",'w').write(generate_string(10));
file3 = open("file3",'w').write(generate_string(10));

a = random.randint(1,42);
b = random.randint(1,42);
sys.stdout.write(str(a)+"\n"+str(b)+"\n"+str(a*b)+"\n");
