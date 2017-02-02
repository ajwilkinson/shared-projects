# shared-projects
## hardware
Hardware related projects
### cg400
32 bit Linux support drivers and sample for the CG400 signal generator PCI card. The original driver bundle can be found [here](
https://web.archive.org/web/20140911225434/http://chase-scientific.com/cg400/CG400_Drivers.zip) - there is little to no licensing descriptions within any of the files, nor any obvious restrictions. 

This has been "made working" on 4.x kernels with a number of cleanups around removed kernel APIs and out of date declarations, and in a few places some bug fixes that were flagged by the newer compilers.

install.sh uses sudo to create the correct nodes and install the driver. setfrequency then takes 1 parameter as the frequency in Mhz to set the card to.
