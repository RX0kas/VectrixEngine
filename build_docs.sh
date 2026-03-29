doxygen ./Doxyfile
cd ./docs || exit
python3 -m http.server 8000