#include "com_fcl_jni_PdfiumJNI.h"
#include "include/fpdfview.h"
#include <string>
#include <cstring>

// Macros para conversión de handles
#define TO_FPDF_DOCUMENT(ptr) reinterpret_cast<FPDF_DOCUMENT>(ptr)
#define TO_FPDF_PAGE(ptr) reinterpret_cast<FPDF_PAGE>(ptr)
#define TO_FPDF_BITMAP(ptr) reinterpret_cast<FPDF_BITMAP>(ptr)
#define TO_JLONG(ptr) reinterpret_cast<jlong>(ptr)

// ========== Funciones de Utilidad ==========

/**
 * Convierte un jstring a const char* UTF-8
 */
const char* jstringToChar(JNIEnv* env, jstring jStr) {
    if (jStr == nullptr) return nullptr;
    return env->GetStringUTFChars(jStr, nullptr);
}

/**
 * Libera un string convertido
 */
void releaseString(JNIEnv* env, jstring jStr, const char* cStr) {
    if (jStr != nullptr && cStr != nullptr) {
        env->ReleaseStringUTFChars(jStr, cStr);
    }
}

// ========== Inicialización y Destrucción ==========

JNIEXPORT void JNICALL Java_com_fcl_jni_PdfiumJNI_initLibrary
  (JNIEnv* env, jclass clazz) {
    FPDF_InitLibrary();
}

JNIEXPORT void JNICALL Java_com_fcl_jni_PdfiumJNI_destroyLibrary
  (JNIEnv* env, jclass clazz) {
    FPDF_DestroyLibrary();
}

// ========== Carga de Documentos ==========

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_loadDocument
  (JNIEnv* env, jclass clazz, jstring filePath, jstring password) {
    
    const char* cFilePath = jstringToChar(env, filePath);
    const char* cPassword = jstringToChar(env, password);
    
    FPDF_DOCUMENT document = FPDF_LoadDocument(cFilePath, cPassword);
    
    releaseString(env, filePath, cFilePath);
    releaseString(env, password, cPassword);
    
    return TO_JLONG(document);
}

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_loadMemDocument___3BLjava_lang_String_2
  (JNIEnv* env, jclass clazz, jbyteArray data, jstring password) {
    
    if (data == nullptr) {
        return 0;
    }
    
    jsize dataLen = env->GetArrayLength(data);
    jbyte* dataPtr = env->GetByteArrayElements(data, nullptr);
    
    const char* cPassword = jstringToChar(env, password);
    
    FPDF_DOCUMENT document = FPDF_LoadMemDocument(dataPtr, dataLen, cPassword);
    
    releaseString(env, password, cPassword);
    env->ReleaseByteArrayElements(data, dataPtr, JNI_ABORT);
    
    return TO_JLONG(document);
}

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_loadMemDocument__Ljava_nio_ByteBuffer_2Ljava_lang_String_2
  (JNIEnv* env, jclass clazz, jobject buffer, jstring password) {
    
    if (buffer == nullptr) {
        return 0;
    }
    
    void* bufferPtr = env->GetDirectBufferAddress(buffer);
    jlong capacity = env->GetDirectBufferCapacity(buffer);
    
    if (bufferPtr == nullptr || capacity <= 0) {
        return 0;
    }
    
    const char* cPassword = jstringToChar(env, password);
    
    FPDF_DOCUMENT document = FPDF_LoadMemDocument(bufferPtr, static_cast<int>(capacity), cPassword);
    
    releaseString(env, password, cPassword);
    
    return TO_JLONG(document);
}

JNIEXPORT void JNICALL Java_com_fcl_jni_PdfiumJNI_closeDocument
  (JNIEnv* env, jclass clazz, jlong document) {
    
    if (document == 0) return;
    
    FPDF_CloseDocument(TO_FPDF_DOCUMENT(document));
}

// ========== Información del Documento ==========

JNIEXPORT jint JNICALL Java_com_fcl_jni_PdfiumJNI_getPageCount
  (JNIEnv* env, jclass clazz, jlong document) {
    
    if (document == 0) return 0;
    
    return FPDF_GetPageCount(TO_FPDF_DOCUMENT(document));
}

JNIEXPORT jint JNICALL Java_com_fcl_jni_PdfiumJNI_getFileVersion
  (JNIEnv* env, jclass clazz, jlong document) {
    
    if (document == 0) return -1;
    
    int fileVersion = 0;
    if (FPDF_GetFileVersion(TO_FPDF_DOCUMENT(document), &fileVersion)) {
        return fileVersion;
    }
    return -1;
}

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_getDocPermissions
  (JNIEnv* env, jclass clazz, jlong document) {
    
    if (document == 0) return 0;
    
    return static_cast<jlong>(FPDF_GetDocPermissions(TO_FPDF_DOCUMENT(document)));
}

JNIEXPORT jint JNICALL Java_com_fcl_jni_PdfiumJNI_getLastError
  (JNIEnv* env, jclass clazz) {
    
    return static_cast<jint>(FPDF_GetLastError());
}

// ========== Operaciones con Páginas ==========

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_loadPage
  (JNIEnv* env, jclass clazz, jlong document, jint pageIndex) {
    
    if (document == 0) return 0;
    
    FPDF_PAGE page = FPDF_LoadPage(TO_FPDF_DOCUMENT(document), pageIndex);
    
    return TO_JLONG(page);
}

JNIEXPORT void JNICALL Java_com_fcl_jni_PdfiumJNI_closePage
  (JNIEnv* env, jclass clazz, jlong page) {
    
    if (page == 0) return;
    
    FPDF_ClosePage(TO_FPDF_PAGE(page));
}

JNIEXPORT jdouble JNICALL Java_com_fcl_jni_PdfiumJNI_getPageWidth
  (JNIEnv* env, jclass clazz, jlong page) {
    
    if (page == 0) return 0.0;
    
    return FPDF_GetPageWidth(TO_FPDF_PAGE(page));
}

JNIEXPORT jdouble JNICALL Java_com_fcl_jni_PdfiumJNI_getPageHeight
  (JNIEnv* env, jclass clazz, jlong page) {
    
    if (page == 0) return 0.0;
    
    return FPDF_GetPageHeight(TO_FPDF_PAGE(page));
}

JNIEXPORT jdoubleArray JNICALL Java_com_fcl_jni_PdfiumJNI_getPageSizeByIndex
  (JNIEnv* env, jclass clazz, jlong document, jint pageIndex) {
    
    if (document == 0) return nullptr;
    
    double width = 0.0;
    double height = 0.0;
    
    int result = FPDF_GetPageSizeByIndex(TO_FPDF_DOCUMENT(document), pageIndex, &width, &height);
    
    if (result == 0) return nullptr;
    
    jdoubleArray jArray = env->NewDoubleArray(2);
    if (jArray == nullptr) return nullptr;
    
    jdouble temp[2] = {width, height};
    env->SetDoubleArrayRegion(jArray, 0, 2, temp);
    
    return jArray;
}

// ========== Renderizado - Bitmaps ==========

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_createBitmap
  (JNIEnv* env, jclass clazz, jint width, jint height, jboolean alpha) {
    
    if (width <= 0 || height <= 0) return 0;
    
    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, alpha ? 1 : 0);
    
    return TO_JLONG(bitmap);
}

JNIEXPORT jlong JNICALL Java_com_fcl_jni_PdfiumJNI_createBitmapEx
  (JNIEnv* env, jclass clazz, jint width, jint height, jint format) {
    
    if (width <= 0 || height <= 0) return 0;
    
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(width, height, format, nullptr, 0);
    
    return TO_JLONG(bitmap);
}

JNIEXPORT void JNICALL Java_com_fcl_jni_PdfiumJNI_destroyBitmap
  (JNIEnv* env, jclass clazz, jlong bitmap) {
    
    if (bitmap == 0) return;
    
    FPDFBitmap_Destroy(TO_FPDF_BITMAP(bitmap));
}

JNIEXPORT jboolean JNICALL Java_com_fcl_jni_PdfiumJNI_fillRect
  (JNIEnv* env, jclass clazz, jlong bitmap, jint left, jint top, jint width, jint height, jint color) {
    
    if (bitmap == 0) return JNI_FALSE;
    
    FPDF_BOOL result = FPDFBitmap_FillRect(TO_FPDF_BITMAP(bitmap), left, top, width, height, 
                                           static_cast<FPDF_DWORD>(color));
    
    return result ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jobject JNICALL Java_com_fcl_jni_PdfiumJNI_getBitmapBuffer
  (JNIEnv* env, jclass clazz, jlong bitmap) {
    
    if (bitmap == 0) return nullptr;
    
    void* buffer = FPDFBitmap_GetBuffer(TO_FPDF_BITMAP(bitmap));
    if (buffer == nullptr) return nullptr;
    
    int width = FPDFBitmap_GetWidth(TO_FPDF_BITMAP(bitmap));
    int height = FPDFBitmap_GetHeight(TO_FPDF_BITMAP(bitmap));
    int stride = FPDFBitmap_GetStride(TO_FPDF_BITMAP(bitmap));
    
    jlong capacity = static_cast<jlong>(stride) * height;
    
    return env->NewDirectByteBuffer(buffer, capacity);
}

JNIEXPORT jint JNICALL Java_com_fcl_jni_PdfiumJNI_getBitmapStride
  (JNIEnv* env, jclass clazz, jlong bitmap) {
    
    if (bitmap == 0) return 0;
    
    return FPDFBitmap_GetStride(TO_FPDF_BITMAP(bitmap));
}

JNIEXPORT void JNICALL Java_com_fcl_jni_PdfiumJNI_renderPageBitmap
  (JNIEnv* env, jclass clazz, jlong bitmap, jlong page, 
   jint startX, jint startY, jint sizeX, jint sizeY, jint rotate, jint flags) {
    
    if (bitmap == 0 || page == 0) return;
    
    FPDF_RenderPageBitmap(TO_FPDF_BITMAP(bitmap), TO_FPDF_PAGE(page),
                         startX, startY, sizeX, sizeY, rotate, flags);
}

// ========== Conversión de Coordenadas ==========

JNIEXPORT jdoubleArray JNICALL Java_com_fcl_jni_PdfiumJNI_deviceToPage
  (JNIEnv* env, jclass clazz, jlong page, jint startX, jint startY, 
   jint sizeX, jint sizeY, jint rotate, jint deviceX, jint deviceY) {
    
    if (page == 0) return nullptr;
    
    double pageX = 0.0;
    double pageY = 0.0;
    
    FPDF_BOOL result = FPDF_DeviceToPage(TO_FPDF_PAGE(page), startX, startY, 
                                         sizeX, sizeY, rotate, deviceX, deviceY, 
                                         &pageX, &pageY);
    
    if (!result) return nullptr;
    
    jdoubleArray jArray = env->NewDoubleArray(2);
    if (jArray == nullptr) return nullptr;
    
    jdouble temp[2] = {pageX, pageY};
    env->SetDoubleArrayRegion(jArray, 0, 2, temp);
    
    return jArray;
}

JNIEXPORT jintArray JNICALL Java_com_fcl_jni_PdfiumJNI_pageToDevice
  (JNIEnv* env, jclass clazz, jlong page, jint startX, jint startY, 
   jint sizeX, jint sizeY, jint rotate, jdouble pageX, jdouble pageY) {
    
    if (page == 0) return nullptr;
    
    int deviceX = 0;
    int deviceY = 0;
    
    FPDF_BOOL result = FPDF_PageToDevice(TO_FPDF_PAGE(page), startX, startY, 
                                         sizeX, sizeY, rotate, pageX, pageY, 
                                         &deviceX, &deviceY);
    
    if (!result) return nullptr;
    
    jintArray jArray = env->NewIntArray(2);
    if (jArray == nullptr) return nullptr;
    
    jint temp[2] = {deviceX, deviceY};
    env->SetIntArrayRegion(jArray, 0, 2, temp);
    
    return jArray;
}