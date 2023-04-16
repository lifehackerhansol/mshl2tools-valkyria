#: <<"#BUILD_R32_LIBELM"
rm mshl2tools_valkyria_libelm.7z
#rm mshl2tools_internal_valkyria_libelm.7z
rm -f /opt/devkitPro
ln -s /opt/devkitPro_r34 /opt/devkitPro
echo "#define LIBELM 1" >libvalkyria/fatdriver.h
LIBELM=1 rebuildcore.sh
makeclean.sh
makeall.sh
makeclean.sh
mshl2tools.sh
rm -f libvalkyria/fatdriver.h
mv mshl2tools_valkyria.7z mshl2tools_valkyria_libelm.7z
#mv mshl2tools_internal.7z mshl2tools_internal_valkyria_libelm.7z
#BUILD_R32_LIBELM

#: <<"#BUILD_R32_LIBFAT"
rm mshl2tools_valkyria_libfat.7z
#rm mshl2tools_internal_valkyria_libfat.7z
rm -f /opt/devkitPro
ln -s /opt/devkitPro_r34 /opt/devkitPro
echo "#define LIBFAT 1" >libvalkyria/fatdriver.h
LIBFAT=1 rebuildcore.sh
makeclean.sh
makeall.sh
makeclean.sh
mshl2tools.sh
rm -f libvalkyria/fatdriver.h
mv mshl2tools_valkyria.7z mshl2tools_valkyria_libfat.7z
#mv mshl2tools_internal.7z mshl2tools_internal_valkyria_libfat.7z
#BUILD_R32_LIBFAT

