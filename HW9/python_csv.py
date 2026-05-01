



# plt.plot(dataD,'g-*')
# plt.xlabel('Time [s]')
# plt.ylabel('Signal')
# plt.title('Signal vs Time')
# plt.show()


import csv
import matplotlib.pyplot as plt # for plotting
import numpy as np # for sine function

def moving_avg_filter(num_points, data):
    averages = []
    for i in range(len(data)-num_points+1):
        subset = data[i:i+num_points]
        current_average = sum(subset) / num_points
        averages.append(current_average)
    return averages


def FFT(data_raw, data_filt):
    Fs = 10000 # sample rate
    Ts = 1.0/Fs; # sampling interval
    # ts = np.arange(0,t[-1],Ts) # time vector
    y_raw = data_raw # the data to make the fft from
    n_raw = len(y_raw) # length of the signal
    k_raw = np.arange(n_raw)
    T_raw = n_raw/Fs
    frq_raw = k_raw/T_raw # two sides frequency range
    frq_raw = frq_raw[range(int(n_raw/2))] # one side frequency range
    Y_raw = np.fft.fft(y_raw)/n_raw # fft computing and normalization
    Y_raw = Y_raw[range(int(n_raw/2))]

    Fs = 10000 # sample rate
    Ts = 1.0/Fs; # sampling interval
    # ts = np.arange(0,t[-1],Ts) # time vector
    y_filt = data_filt # the data to make the fft from
    n_filt = len(y_filt) # length of the signal
    k_filt = np.arange(n_filt)
    T_filt = n_filt/Fs
    frq_filt = k_filt/T_filt # two sides frequency range
    frq_filt = frq_filt[range(int(n_filt/2))] # one side frequency range
    Y_filt = np.fft.fft(y_filt)/n_filt # fft computing and normalization
    Y_filt = Y_filt[range(int(n_filt/2))]

    fig, (ax1) = plt.subplots(1, 1)
    ax1.loglog(frq_raw,abs(Y_raw),'k')
    ax1.loglog(frq_filt,abs(Y_filt),'r') # plotting the fft
    ax1.set_xlabel('Freq (Hz)')
    ax1.set_ylabel('|Y(freq)|')
    ax1.set_title('FFT SigD FIR: cutoff=46Hz, bL=24Hz, 77 coeffs')
    plt.show()



def IIR(A,B, data):
    averages = [data[0]]
    for i in range(1,len(data)):
        new_average = A*averages[i-1] + B*data[i]
        averages.append(new_average)
    return averages


def FIR(coeffs, data):
    averages = []
    for i in range(len(data)-len(coeffs)+1):
        subset = data[i:i+len(coeffs)]
        sum = 0
        for j in range(len(coeffs)):
            sum = sum + subset[j]*coeffs[j]
        averages.append(sum)
    return averages


# read sigA
t = [] # column 0
dataA = [] # column 1
with open('sigA.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        dataA.append(float(row[1])) # second column

sample_rateA = len(t) / t[-1]
print("SAMPLE RATE A: ")
print(sample_rateA)

# read sigB
t = [] # column 0
dataB = [] # column 1
with open('sigB.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        dataB.append(float(row[1])) # second column

sample_rateB = len(t) / t[-1]
print("SAMPLE RATE B: ")
print(sample_rateB)


# read sigC
t = [] # column 0
dataC = [] # column 1
with open('sigC.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        dataC.append(float(row[1])) # second column

sample_rateC = len(t) / t[-1]
print("SAMPLE RATE C: ")
print(sample_rateC)

# read sigD
t = [] # column 0
dataD = [] # column 1
with open('sigD.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        dataD.append(float(row[1])) # second column

sample_rateD = len(t) / t[-1]
print("SAMPLE RATE D: ")
print(sample_rateD)

# apply moving average filter
filteredA = moving_avg_filter(700,dataA)
filteredB = moving_avg_filter(200,dataB)
filteredC = moving_avg_filter(100,dataC)
filteredD = moving_avg_filter(50,dataD)

# apply IIR filter
iirA = IIR(0.995,0.005,dataA)
iirB = IIR(0.99,0.01,dataB)
iirC = IIR(0.95,0.05,dataC)
iirD = IIR(0.9,0.1,dataD)

# apply FIR filter
coeffsA = [
    0.000000000000000000,
    0.000014268081979019,
    0.000061124985674506,
    0.000147662239939894,
    0.000282452978580434,
    0.000475622294069235,
    0.000738789260786917,
    0.001084869029756778,
    0.001527731615395616,
    0.002081722813376540,
    0.002761061611552014,
    0.003579136968041662,
    0.004547734426911014,
    0.005676229256038261,
    0.006970787223567474,
    0.008433616468633741,
    0.010062313968020859,
    0.011849347775034913,
    0.013781711561576353,
    0.015840781209617145,
    0.018002394575832255,
    0.020237165501582798,
    0.022511032154490480,
    0.024786028422533708,
    0.027021255922960386,
    0.029174023821897085,
    0.031201114639638674,
    0.033060127031954663,
    0.034710841590318654,
    0.036116553282452370,
    0.037245314418832491,
    0.038071035002182958,
    0.038574392876111516,
    0.038743513981319019,
    0.038574392876111516,
    0.038071035002182972,
    0.037245314418832498,
    0.036116553282452370,
    0.034710841590318661,
    0.033060127031954677,
    0.031201114639638670,
    0.029174023821897089,
    0.027021255922960379,
    0.024786028422533708,
    0.022511032154490494,
    0.020237165501582798,
    0.018002394575832258,
    0.015840781209617145,
    0.013781711561576359,
    0.011849347775034918,
    0.010062313968020859,
    0.008433616468633750,
    0.006970787223567470,
    0.005676229256038261,
    0.004547734426911019,
    0.003579136968041662,
    0.002761061611552016,
    0.002081722813376543,
    0.001527731615395616,
    0.001084869029756781,
    0.000738789260786920,
    0.000475622294069236,
    0.000282452978580436,
    0.000147662239939894,
    0.000061124985674507,
    0.000014268081979020,
    0.000000000000000000,
]
firA = FIR(coeffsA, dataA)

coeffsB = [
    0.000000000000000000,
    0.000014268081979019,
    0.000061124985674506,
    0.000147662239939894,
    0.000282452978580434,
    0.000475622294069235,
    0.000738789260786917,
    0.001084869029756778,
    0.001527731615395616,
    0.002081722813376540,
    0.002761061611552014,
    0.003579136968041662,
    0.004547734426911014,
    0.005676229256038261,
    0.006970787223567474,
    0.008433616468633741,
    0.010062313968020859,
    0.011849347775034913,
    0.013781711561576353,
    0.015840781209617145,
    0.018002394575832255,
    0.020237165501582798,
    0.022511032154490480,
    0.024786028422533708,
    0.027021255922960386,
    0.029174023821897085,
    0.031201114639638674,
    0.033060127031954663,
    0.034710841590318654,
    0.036116553282452370,
    0.037245314418832491,
    0.038071035002182958,
    0.038574392876111516,
    0.038743513981319019,
    0.038574392876111516,
    0.038071035002182972,
    0.037245314418832498,
    0.036116553282452370,
    0.034710841590318661,
    0.033060127031954677,
    0.031201114639638670,
    0.029174023821897089,
    0.027021255922960379,
    0.024786028422533708,
    0.022511032154490494,
    0.020237165501582798,
    0.018002394575832258,
    0.015840781209617145,
    0.013781711561576359,
    0.011849347775034918,
    0.010062313968020859,
    0.008433616468633750,
    0.006970787223567470,
    0.005676229256038261,
    0.004547734426911019,
    0.003579136968041662,
    0.002761061611552016,
    0.002081722813376543,
    0.001527731615395616,
    0.001084869029756781,
    0.000738789260786920,
    0.000475622294069236,
    0.000282452978580436,
    0.000147662239939894,
    0.000061124985674507,
    0.000014268081979020,
    0.000000000000000000,
]
firB = FIR(coeffsB, dataB)


coeffsC = [
    0.000000000000000000,
    0.000006654468709502,
    0.000029128522936023,
    0.000071630294094485,
    0.000139045364715286,
    0.000237018146420107,
    0.000371990776314115,
    0.000551191427798929,
    0.000782566139698271,
    0.001074650878794300,
    0.001436383452247726,
    0.001876857948081446,
    0.002405027463171737,
    0.003029363833662816,
    0.003757485769150283,
    0.004595769074907365,
    0.005548954406576212,
    0.006619769140808002,
    0.007808580390891126,
    0.009113095905728782,
    0.010528128553079571,
    0.012045438326481696,
    0.013653663385294468,
    0.015338348625262888,
    0.017082076796941639,
    0.018864703378109852,
    0.020663692418049256,
    0.022454546570933227,
    0.024211320690754528,
    0.025907204835505290,
    0.027515159476891312,
    0.029008583268927898,
    0.030361992005490210,
    0.031551686475304340,
    0.032556386852028101,
    0.033357812050676970,
    0.033941184117168982,
    0.034295640137166114,
    0.034414537262454484,
    0.034295640137166114,
    0.033941184117168982,
    0.033357812050676984,
    0.032556386852028101,
    0.031551686475304347,
    0.030361992005490220,
    0.029008583268927898,
    0.027515159476891322,
    0.025907204835505290,
    0.024211320690754531,
    0.022454546570933234,
    0.020663692418049256,
    0.018864703378109866,
    0.017082076796941632,
    0.015338348625262895,
    0.013653663385294473,
    0.012045438326481693,
    0.010528128553079576,
    0.009113095905728785,
    0.007808580390891126,
    0.006619769140808005,
    0.005548954406576217,
    0.004595769074907367,
    0.003757485769150290,
    0.003029363833662813,
    0.002405027463171739,
    0.001876857948081447,
    0.001436383452247725,
    0.001074650878794302,
    0.000782566139698270,
    0.000551191427798929,
    0.000371990776314116,
    0.000237018146420106,
    0.000139045364715286,
    0.000071630294094485,
    0.000029128522936022,
    0.000006654468709502,
    0.000000000000000000,
]
firC = FIR(coeffsC, dataC)

coeffsD = [
    0.000000000000000000,
    0.000001953017685321,
    0.000019879743400474,
    0.000048938680721420,
    0.000046108101439211,
    -0.000039092229771804,
    -0.000201736550558458,
    -0.000338430305312557,
    -0.000281086672829440,
    0.000080817157329917,
    0.000654815326039793,
    0.001097282703502050,
    0.000958711746702255,
    -0.000000000000000001,
    -0.001497839754478390,
    -0.002686861794916530,
    -0.002529645118952172,
    -0.000497827652400597,
    0.002804731472847875,
    0.005595420527554871,
    0.005709572584511651,
    0.001986696165981717,
    -0.004527619440425633,
    -0.010479344747285752,
    -0.011658239733482421,
    -0.005536595171479415,
    0.006458121579402466,
    0.018524577191090050,
    0.022767375101632716,
    0.013537173939776036,
    -0.008254902848520802,
    -0.033431816921974847,
    -0.047084810492700253,
    -0.034879137841780254,
    0.009535668876549008,
    0.079718336132024351,
    0.157075973452364487,
    0.217297644258735184,
    0.240010379035157095,
    0.217297644258735184,
    0.157075973452364487,
    0.079718336132024364,
    0.009535668876549008,
    -0.034879137841780254,
    -0.047084810492700267,
    -0.033431816921974847,
    -0.008254902848520805,
    0.013537173939776036,
    0.022767375101632719,
    0.018524577191090057,
    0.006458121579402466,
    -0.005536595171479418,
    -0.011658239733482418,
    -0.010479344747285757,
    -0.004527619440425634,
    0.001986696165981716,
    0.005709572584511655,
    0.005595420527554873,
    0.002804731472847875,
    -0.000497827652400598,
    -0.002529645118952175,
    -0.002686861794916531,
    -0.001497839754478393,
    -0.000000000000000001,
    0.000958711746702256,
    0.001097282703502050,
    0.000654815326039792,
    0.000080817157329917,
    -0.000281086672829439,
    -0.000338430305312557,
    -0.000201736550558459,
    -0.000039092229771804,
    0.000046108101439211,
    0.000048938680721420,
    0.000019879743400474,
    0.000001953017685321,
    0.000000000000000000,
]

firD = FIR(coeffsD, dataD)

# FIR Graph SigA
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataA,'k')
# ax1.plot(firA,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigA FIR: cutoff=100Hz, bL=700Hz, 67 coeffs')
# plt.show()
# FFT(dataA, firA)

# FIR Graph SigB
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataB,'k')
# ax1.plot(firB,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigB FIR: cutoff=33Hz, bL=231Hz, 67 coeffs')
# plt.show()
# FFT(dataB, firB)

# FIR Graph SigC
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataC,'k')
# ax1.plot(firC,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigC FIR: cutoff=25Hz, bL=150Hz, 77 coeffs')
# plt.show()
# FFT(dataC, firC)

# FIR Graph SigD
fig, (ax1) = plt.subplots(1, 1)
ax1.plot(dataD,'k')
ax1.plot(firD,'r')
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax1.set_title('SigD FIR: cutoff=46Hz, bL=24Hz, 77 coeffs')
plt.show()
FFT(dataD, firD)


# ### IIR GRAPHING ###
# # IIR Graph SigA
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataA,'k')
# ax1.plot(iirA,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigA IIR Filter (A=0.995, B=0.005)')
# plt.show()
# FFT(dataA, iirA)

# # IIR Graph SigB
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataB,'k')
# ax1.plot(iirB,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigB IIR Filter (A=0.99, B=0.01)')
# plt.show()
# FFT(dataB, iirB)

# # IIR Graph SigC
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataC,'k')
# ax1.plot(iirC,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigC IIR Filter (A=0.95, B=0.05)')
# plt.show()
# FFT(dataC, iirC)

# # IIR Graph SigD
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataD,'k')
# ax1.plot(iirD,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigD IIR Filter (A=0.9, B=0.1)')
# plt.show()
# FFT(dataD, iirD)

#### MOVING AVERAGE GRAPHING ###
# # Graph SigA Unfiltered and Filtered One Plot
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataA,'k')
# ax1.plot(filteredA,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigA Moving Avg Filter, 700 Points')
# plt.show()

# FFT(dataA, filteredA)

# # Graph SigB Unfiltered and Filtered One Plot
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataB,'k')
# ax1.plot(filteredB,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigB Moving Avg Filter, 200 Points')
# plt.show()

# FFT(dataB, filteredB)

# # Graph SigC Unfiltered and Filtered One Plot
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataC,'k')
# ax1.plot(filteredC,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigC Moving Avg Filter, 100 Points')
# plt.show()

# FFT(dataC, filteredC)

# # Graph SigD Unfiltered and Filtered One Plot
# fig, (ax1) = plt.subplots(1, 1)
# ax1.plot(dataD,'k')
# ax1.plot(filteredD,'r')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax1.set_title('SigD Moving Avg Filter, 50 Points')
# plt.show()

# FFT(dataD, filteredD)


