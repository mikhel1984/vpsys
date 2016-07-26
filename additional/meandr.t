# meandr.t
"Meandr generator" # type description

... PAR
# parameters

amp, t # define amplitude and period
amp: "meandr amplitude" # description
amp: val = 1 # default amplitude
t: val = %pi # default period

... OUT
# output connectors

out0: type = number # output for single number (default)

... FUNC
# data processing

# use sinusoid and 'tau' - time variable

k = 2*%pi / t # internal variable

IF sin(k * tau) > 0 THEN
   out0 = amp
ELSE
   out0 = 0
END

