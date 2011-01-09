@rem
@rem Simple batch file to generate the needed .h,.c files
@rem for building with an IDE like VS or Keil
@rem
@rem Assumes python.exe is on the path

@rem build the pmfeatures.h
python ../../tools/pmGenPmFeatures.py pmfeatures.py > pmfeatures.h

@rem build the 'system' libraries
python ../../tools/pmImgCreator.py -f pmfeatures.py -c -s -o ../../vm/pmstdlib_img.c --native-file=../../vm/pmstdlib_nat.c ../../lib/ipm.py ../../lib/list.py ../../lib/dict.py ../../lib/__bi.py ../../lib/sys.py ../../lib/string.py

@rem build the 'local' libraries
python ../../tools/pmImgCreator.py -f pmfeatures.py -c -u -o main_img.c --native-file=main_nat.c main.py 

@rem build the pmfeatures.h file
python ../../tools/pmGenPmFeatures.py pmfeatures.py > pmfeatures.h
