f = open('debug.dat', 'r')

i = 0
for line in f:
    if i == 0 or i == 1 or i == 2:
        i += 1
        continue

    if line == '\n':
        break

    arr = line.rstrip('\n').split()[3:]
    print '{0:016b} {1:016b}'.format(int(arr[0], 16), int(arr[1], 16))

    i += 1
