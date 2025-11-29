package com.fcl.demo;

import com.fcl.jni.PdfiumJNI;

import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Prueba de concepto para visualizar PDFs usando PDFium JNI
 */
public class PdfViewerDemo extends JFrame {
    
    private JLabel imageLabel;
    private JPanel controlPanel;
    private JButton prevButton, nextButton;
    private JLabel pageLabel;
    
    private long document = 0;
    private int currentPage = 0;
    private int totalPages = 0;
    private int renderWidth = 800;
    private int renderHeight = 1000;
    
    public PdfViewerDemo() {
        setTitle("PDFium Viewer - Demo");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(850, 1100);
        setLocationRelativeTo(null);
        
        initComponents();
        initPdfium();
    }
    
    private void initComponents() {
        // Panel principal con scroll
        imageLabel = new JLabel();
        imageLabel.setHorizontalAlignment(JLabel.CENTER);
        JScrollPane scrollPane = new JScrollPane(imageLabel);
        
        // Panel de controles
        controlPanel = new JPanel();
        prevButton = new JButton("◄ Anterior");
        nextButton = new JButton("Siguiente ►");
        pageLabel = new JLabel("Página: 0/0");
        
        prevButton.addActionListener(e -> previousPage());
        nextButton.addActionListener(e -> nextPage());
        
        JButton openButton = new JButton("Abrir PDF");
        openButton.addActionListener(e -> openPdf());
        
        controlPanel.add(openButton);
        controlPanel.add(prevButton);
        controlPanel.add(pageLabel);
        controlPanel.add(nextButton);
        
        // Layout
        setLayout(new BorderLayout());
        add(scrollPane, BorderLayout.CENTER);
        add(controlPanel, BorderLayout.SOUTH);
        
        updateButtons();
    }
    
    private void initPdfium() {
        try {
            PdfiumJNI.initLibrary();
            System.out.println("PDFium inicializado correctamente");
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                "Error al inicializar PDFium: " + e.getMessage(),
                "Error", JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
        }
    }
    
    private void openPdf() {
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setFileFilter(new javax.swing.filechooser.FileFilter() {
            public boolean accept(File f) {
                return f.isDirectory() || f.getName().toLowerCase().endsWith(".pdf");
            }
            public String getDescription() {
                return "Archivos PDF (*.pdf)";
            }
        });
        
        if (fileChooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            File selectedFile = fileChooser.getSelectedFile();
            loadPdf(selectedFile.getAbsolutePath());
        }
    }
    
    private void loadPdf(String filePath) {
        // Cerrar documento anterior si existe
        if (document != 0) {
            PdfiumJNI.closeDocument(document);
        }
        
        // Cargar nuevo documento
        document = PdfiumJNI.loadDocument(filePath, null);
        
        if (document == 0) {
            int error = PdfiumJNI.getLastError();
            String errorMsg = getErrorMessage(error);
            JOptionPane.showMessageDialog(this,
                "Error al cargar PDF: " + errorMsg,
                "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        totalPages = PdfiumJNI.getPageCount(document);
        currentPage = 0;
        
        System.out.println("PDF cargado: " + filePath);
        System.out.println("Total de páginas: " + totalPages);
        
        renderCurrentPage();
        updateButtons();
    }
    
    private void renderCurrentPage() {
        if (document == 0 || totalPages == 0) {
            return;
        }
        
        // Cargar página
        long page = PdfiumJNI.loadPage(document, currentPage);
        if (page == 0) {
            JOptionPane.showMessageDialog(this,
                "Error al cargar la página " + (currentPage + 1),
                "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        try {
            // Obtener dimensiones de la página
            double pageWidth = PdfiumJNI.getPageWidth(page);
            double pageHeight = PdfiumJNI.getPageHeight(page);
            
            // Calcular escala para ajustar al ancho deseado
            double scale = renderWidth / pageWidth;
            int scaledHeight = (int)(pageHeight * scale);
            
            // Crear bitmap
            long bitmap = PdfiumJNI.createBitmap(renderWidth, scaledHeight, true);
            if (bitmap == 0) {
                JOptionPane.showMessageDialog(this,
                    "Error al crear bitmap",
                    "Error", JOptionPane.ERROR_MESSAGE);
                return;
            }
            
            try {
                // Llenar con blanco
                PdfiumJNI.fillRect(bitmap, 0, 0, renderWidth, scaledHeight, 0xFFFFFFFF);
                
                // Renderizar página
                PdfiumJNI.renderPageBitmap(
                    bitmap, page,
                    0, 0, renderWidth, scaledHeight,
                    PdfiumJNI.ROTATE_0,
                    PdfiumJNI.FPDF_ANNOT | PdfiumJNI.FPDF_LCD_TEXT
                );
                
                // Convertir bitmap a imagen Java
                BufferedImage image = bitmapToBufferedImage(bitmap, renderWidth, scaledHeight);
                
                // Mostrar imagen
                imageLabel.setIcon(new ImageIcon(image));
                
                System.out.println("Página " + (currentPage + 1) + " renderizada " +
                    "(" + renderWidth + "x" + scaledHeight + ")");
                
            } finally {
                PdfiumJNI.destroyBitmap(bitmap);
            }
            
        } finally {
            PdfiumJNI.closePage(page);
        }
        
        pageLabel.setText("Página: " + (currentPage + 1) + "/" + totalPages);
    }
    
    private BufferedImage bitmapToBufferedImage(long bitmap, int width, int height) {
        // Obtener buffer del bitmap
        ByteBuffer buffer = PdfiumJNI.getBitmapBuffer(bitmap);
        int stride = PdfiumJNI.getBitmapStride(bitmap);
        
        // Crear BufferedImage
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        int[] pixels = ((DataBufferInt) image.getRaster().getDataBuffer()).getData();
        
        // PDFium usa formato BGRA, convertir a ARGB
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int offset = y * stride + x * 4;
                
                int b = buffer.get(offset) & 0xFF;
                int g = buffer.get(offset + 1) & 0xFF;
                int r = buffer.get(offset + 2) & 0xFF;
                int a = buffer.get(offset + 3) & 0xFF;
                
                // Convertir BGRA a ARGB
                pixels[y * width + x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
        
        return image;
    }
    
    private void previousPage() {
        if (currentPage > 0) {
            currentPage--;
            renderCurrentPage();
            updateButtons();
        }
    }
    
    private void nextPage() {
        if (currentPage < totalPages - 1) {
            currentPage++;
            renderCurrentPage();
            updateButtons();
        }
    }
    
    private void updateButtons() {
        prevButton.setEnabled(currentPage > 0);
        nextButton.setEnabled(currentPage < totalPages - 1);
    }
    
    private String getErrorMessage(int errorCode) {
        switch (errorCode) {
            case PdfiumJNI.FPDF_ERR_SUCCESS: return "Sin error";
            case PdfiumJNI.FPDF_ERR_UNKNOWN: return "Error desconocido";
            case PdfiumJNI.FPDF_ERR_FILE: return "Archivo no encontrado";
            case PdfiumJNI.FPDF_ERR_FORMAT: return "Formato PDF inválido";
            case PdfiumJNI.FPDF_ERR_PASSWORD: return "Contraseña requerida o incorrecta";
            case PdfiumJNI.FPDF_ERR_SECURITY: return "Esquema de seguridad no soportado";
            case PdfiumJNI.FPDF_ERR_PAGE: return "Página no encontrada";
            default: return "Error código: " + errorCode;
        }
    }
    
    @Override
    public void dispose() {
        // Limpiar recursos
        if (document != 0) {
            PdfiumJNI.closeDocument(document);
            document = 0;
        }
        
        PdfiumJNI.destroyLibrary();
        System.out.println("PDFium finalizado");
        
        super.dispose();
    }
    
    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            PdfViewerDemo viewer = new PdfViewerDemo();
            viewer.setVisible(true);
        });
    }
}