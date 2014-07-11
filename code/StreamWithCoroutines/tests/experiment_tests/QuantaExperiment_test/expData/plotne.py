import matplotlib.pyplot as plt
import numpy as np
import pylab

inFile = "ChainExperiment-selectivity.dat"

f = open(inFile, 'r')

#haci benim datada bi satirda 3 veri okumam gerekmis o yuzden x y z gibi amele cinsinden yazdim nasilsa ciziyor atraksiyona girmeye gerek yok demistim
#sen bir struct falan kullanabilirsin ya da amelelige devam :P
f.readline()
line = f.readline()
t = line.split()

xLabel = t[1]
line = f.readline()
x = []
t1 = []
t2 = []
t3 = []
while (line) :
    t = line.split()
    x.append(float(t[0]))
    t1.append(float(t[1]))
    t2.append(float(t[2]))
    t3.append(float(t[3]))
    line = f.readline()

x = np.array(x)
t1 = np.array(t1)
t2 = np.array(t2)
t3 = np.array(t3)


#haci simdi burda x x ekseninde y y ekseninde demek
# "bo" blue oval demek "-" ile de noktalari birlestir diyorsun
#label ise legendda ne yazacak onu gosteriyor
plt.plot(x, t1, "bo-", label='random', linewidth=1.0 )

#haci burasi x-y eksenlerinin limitini belirliyor
#plt.axis((,,,))

#ayni plotda iki ayri line cizmek icin bir onceki line i hold ediyoruz
plt.hold(True)
plt.plot(x,t2, "rs-", label='maxThroughput', linewidth=1.0)
plt.hold(True)
plt.plot(x,t3, "g*-", label='minLatency', linewidth=1.0)

#legend nereye konacak x-y ekseninde ne yazacak
plt.legend(loc='upper left', shadow=False)
plt.xlabel(xLabel)
plt.ylabel('throughput')

#plota grid koy ya da koyma
plt.grid()

#hangi adla save edeceksin
plt.savefig(xLabel)
plt.show()
