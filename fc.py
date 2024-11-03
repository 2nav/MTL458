# python code to generate a file of n lines, each saying Bye Bye World
# where n is hardcoded on the next line
n = 100000
with open('shared-filee.txt', 'w') as f:
    for i in range(n):
        f.write('Bye Bye World\n')
#close file?
f.close()
#end of code