#!/bin/bash
set -e

#avr=${avr:=/Users/kel/Desktop/sloeber.app/Contents/Eclipse/arduinoPlugin/packages/arduino/hardware/avr/1.6.20}
avr=${avr:=/Applications/Arduino.app/Contents/Java/hardware/arduino/avr}

port=${port:=/dev/cu.usbmodem1411}

#avrbin=${avrbin:=`}

if [ -z ${avrbin+x} ]; then avrbin=`which avr-gcc`; avrbin=`dirname $avrbin`; else echo "var is set to '$avrbin'";  avrdudeconfig=$avrbin/../etc/avrdude.conf; fi

echo $avrdudeconfig

if [ $# -eq 0 ]
then
	echo "No arguments supplied.  Please specify the path to the C source code FMU."
	exit -1
fi



rm -rf build
mkdir build



echo "Preparing...Main with valueReferences"

cp $1 build/fmu.zip
cd build

unzip -o fmu.zip > /dev/null
rm -f sources/main.c


guid=`grep 'guid' modelDescription.xml | awk -F \" '{print $2}'`

leftval=`grep 'name="lfLeftVal"' modelDescription.xml | awk -F \" '{print $4}'`
forwardrotate=`grep 'name="forwardRotate"' modelDescription.xml | awk -F \" '{print $4}'`
servoleft=`grep 'name="servoLeftVal"' modelDescription.xml | awk -F \" '{print $4}'`
servoright=`grep 'name="servoRightVal"' modelDescription.xml | awk -F \" '{print $4}'`
forwardspeed=`grep 'name="forwardSpeed"' modelDescription.xml | awk -F \" '{print $4}'`
rightval=`grep 'name="lfRightVal"' modelDescription.xml | awk -F \" '{print $4}'`
backwardrotate=`grep 'name="backwardRotate"' modelDescription.xml | awk -F \" '{print $4}'`
threshold=`grep 'name="threshold"' modelDescription.xml | awk -F \" '{print $4}'`

cp ../modeldescription.h .

sed -i'.original' "s/XX_leftval_XX/$leftval/g" modeldescription.h
sed -i'.original' "s/XX_forwardrotate_XX/$forwardrotate/g" modeldescription.h
sed -i'.original' "s/XX_servoleft_XX/$servoleft/g" modeldescription.h
sed -i'.original' "s/XX_servoright_XX/$servoright/g" modeldescription.h
sed -i'.original' "s/XX_forwardspeed_XX/$forwardspeed/g" modeldescription.h
sed -i'.original' "s/XX_rightval_XX/$rightval/g" modeldescription.h
sed -i'.original' "s/XX_backwardrotate_XX/$backwardrotate/g" modeldescription.h
sed -i'.original' "s/XX_threshold_XX/$threshold/g" modeldescription.h
sed -i'.original' "s/XX_guid_XX/$guid/g" modeldescription.h


cp ../main.cpp .

echo "Copying libraries in place"



echo "Compiling..."

MCU=atmega2560
F_CPU=16000000UL
MCU_AVRDUDE=atmega2560

cd sources

echo "Compiling VDM"
for i in `find . -type f -name '*.c'`
do
	${avrbin}/avr-gcc -I"." -I"./fmi" -I"./vdmlib" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=$MCU -DF_CPU=$F_CPU -MMD -MP -MF"$i.d" -MT"$i.o" -c -o "$i.o" "$i"
done

echo "Compiling Arduino"

acore=$avr/cores/arduino

mkdir -p acore
cp $acore/*.cpp acore
cp $acore/*.h acore
cp $acore/*.c acore
cp $acore/*.S acore

for i in `find acore -type f -name '*.c'`
do
	echo $i
	${avrbin}/avr-gcc -I"." -I"${acore}" -I"${avr}/variants/mega" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=$MCU -DF_CPU=$F_CPU -MMD -MP -MF"$i.d" -MT"$i.o" -c -o "$i.o" "$i"
done

for i in `find acore -type f -name '*.S'`
do
	echo $i
	${avrbin}/avr-gcc -I"." -I"${acore}" -I"${avr}/variants/mega" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=$MCU -DF_CPU=$F_CPU -MMD -MP -MF"$i.d" -MT"$i.o" -c -o "$i.o" "$i"
done

for i in `find acore -type f -name '*.cpp'`
do
	echo $i
	${avrbin}/avr-g++ -I"." -I".." -I"./fmi" -I"${acore}" -I"${avr}/variants/mega" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu++11 -funsigned-char -funsigned-bitfields -mmcu=$MCU -DF_CPU=$F_CPU -MMD -MP -MF"$i.d" -MT"$i.o" -c -o "$i.o" "$i"
done

echo "Compiling main"

${avrbin}/avr-g++ -I"./vdmlib" -I"." -I".." -I"./fmi" -I"${acore}" -I"${avr}/variants/mega" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu++11 -funsigned-char -funsigned-bitfields -mmcu=$MCU -DF_CPU=$F_CPU -MMD -MP -MF"$i.d" -MT"mainl.o" -c -o "mainl.o" ../main.cpp > /dev/null

ofiles=`find . -type f -name '*.o'`

${avrbin}/avr-gcc -Wl,-Map,AURobot.map -mmcu=$MCU -o "AURobot.elf"  $ofiles
${avrbin}/avr-objdump -h -S AURobot.elf  > "AURobot.lss"
${avrbin}/avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex AURobot.elf  "AURobot.hex"
${avrbin}/avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex AURobot.elf  "AURobot.eep"
${avrbin}/avr-size --format=avr --mcu=$MCU AURobot.elf


echo "Deploying..."
#Fuse settings:  High:  1D, Low:  C2
#avrdude -v -V -p $MCU_AVRDUDE -c stk500v2 -P /dev/cu.usbmodem1411 -b 115200 -Uflash:w:AURobot.hex:a
#avrdude -p atmega2560  -c stk500v2 -P /dev/ttyACM0 -b 115200 -U "

if [ -z ${avrdudeconfig+x} ]; then 
	${avrbin}/avrdude -F -v -p atmega2560 -c stk500v2 -P $port -b115200 -D  -Uflash:w:AURobot.hex:a ;
else 
	${avrbin}/avrdude -C $avrdudeconfig -F -v -p atmega2560 -c stk500v2 -P $port -b115200 -D  -Uflash:w:AURobot.hex:a ;
fi


echo "Cleaning..."
cd ..
#rm -rf resources sources binaries LFRController.zip modelDescription.xml main.c AURobot.* *.c.d *.c.o



echo "Done."

#minicom -D/dev/cu.usbmodem1411 -b115200
