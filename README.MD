
# Prueba de concepto Visor PDF

En esta prueba de concepto lo unico que se hace es enlazar una biblioteta nativa a traves de un puente JNI para que dicha biblioteca pueda ser usada dentro de un programa Java.

* Se generan los headers a partir de la clase java que va a hacer de puente
``` bash
javac -h . com/fcl/jni/PdfiumJNI.java
```
* Se implementa la definicion (.h) en una clase de c++ con el mismo nombre del archivo de definicion 
* Se compila la clase de c++
```bash
g++ -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux com_fcl_jni_PdfiumJNI.cpp -o com_fcl_jni_PdfiumJNI.o
```
* Se enlaza el compilado como una biblioteca compartida (Shared library)
```bash
g++ -shared -fPIC -o lib/libpdfium-jni.so com_fcl_jni_PdfiumJNI.o -lc
```

* Compilado del demo y de la interfaz JNI
```bash
javac -d bin com/fcl/jni/PdfiumJNI.java
javac -d bin -cp bin com/fcl/demo/PdfViewerDemo.java
```
* Ejecución de la prueba de concepto
```bash
java -Djava.library.path=/lib -cp bin com.fcl.demo.PdfViewerDemo
```