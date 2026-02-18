
def change_freq_lvl(instr, frequency, level):
  instr.write('SOUR:FREQ:CW '+ str(frequency))
  instr.write('SOUR:POW:LEV:IMM:AMPL ' + str(level))

def output(instr, status):
  instr.write('OUTP1:STAT ' + str(status))