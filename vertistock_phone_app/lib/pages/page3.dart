import 'package:flutter/material.dart';
import 'package:mobile_scanner/mobile_scanner.dart';
import 'dart:convert';

class Page3 extends StatefulWidget {
  const Page3({super.key});

  @override
  State<Page3> createState() => _Page3State();
}

class _Page3State extends State<Page3> {
  late MobileScannerController cameraController;
  Map<String, dynamic>? scannedData;
  bool isScanned = false;

  @override
  void initState() {
    super.initState();
    cameraController = MobileScannerController();
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    cameraController
        .start(); // ✅ Riavvia la fotocamera quando la pagina è visibile
  }

  void _onDetect(BarcodeCapture capture) {
    if (capture.barcodes.isEmpty || isScanned) return;

    try {
      final String? rawValue = capture.barcodes.first.rawValue;
      if (rawValue == null) return;

      final Map<String, dynamic> jsonData = json.decode(rawValue);
      setState(() {
        scannedData = jsonData;
        isScanned = true;
      });
    } catch (e) {
      setState(() {
        scannedData = {"Errore": "QR Code non valido"};
      });
    }
  }

  void _resetScanner() {
    setState(() {
      scannedData = null;
      isScanned = false;
    });
    cameraController.start(); // ✅ Riavvia la fotocamera dopo il reset
  }

  @override
  void dispose() {
    cameraController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      body: Stack(
        children: [
          Positioned.fill(
            child: MobileScanner(
              controller: cameraController,
              onDetect: _onDetect,
            ),
          ),
          Positioned(
            top: MediaQuery.of(context).size.height * 0.25,
            left: MediaQuery.of(context).size.width * 0.15,
            child: SizedBox(
              width: MediaQuery.of(context).size.width * 0.7,
              height: MediaQuery.of(context).size.width * 0.7,
              child: CustomPaint(
                painter: CornerPainter(),
              ),
            ),
          ),
          if (isScanned)
            Positioned.fill(
              child: Container(
                color: Colors.black.withOpacity(0.8),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Icon(Icons.check_circle,
                        size: 100, color: Colors.green),
                    const SizedBox(height: 20),
                    Container(
                      margin: const EdgeInsets.symmetric(horizontal: 20),
                      padding: const EdgeInsets.all(16),
                      decoration: BoxDecoration(
                        color: const Color(0xFF0D1D31), // Sfondo scuro
                        borderRadius: BorderRadius.circular(3),
                        border: Border.all(
                          color: const Color(0xFFEC7900), // Bordo arancione 1px
                          width: 1,
                        ),
                        boxShadow: [
                          BoxShadow(
                            color: Colors.black.withOpacity(0.2),
                            blurRadius: 10,
                            spreadRadius: 2,
                          ),
                        ],
                      ),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          _buildRow("Name:", scannedData?["name"]),
                          _buildRow("Code:", scannedData?["code"]),
                          _buildRow("Count:", scannedData?["count"].toString()),
                        ],
                      ),
                    ),
                    const SizedBox(height: 20),
                    ElevatedButton(
                      onPressed: _resetScanner,
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.symmetric(
                            horizontal: 40, vertical: 12),
                        backgroundColor:
                            const Color(0xFF0D1D31), // Sfondo scuro
                        elevation: 0,
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(3),
                          side: const BorderSide(
                            color: Color(0xFFEC7900), // Bordo arancione di 1px
                            width: 1,
                          ),
                        ),
                      ).copyWith(
                        backgroundColor:
                            MaterialStateProperty.resolveWith<Color>(
                          (Set<MaterialState> states) {
                            if (states.contains(MaterialState.pressed)) {
                              return const Color(
                                  0xFFEC7900); // 🔹 Sfondo arancione quando premuto
                            }
                            return const Color(
                                0xFF0D1D31); // 🔹 Sfondo predefinito
                          },
                        ),
                        foregroundColor:
                            MaterialStateProperty.resolveWith<Color>(
                          (Set<MaterialState> states) {
                            if (states.contains(MaterialState.pressed)) {
                              return const Color(
                                  0xFF0D1D31); // 🔹 Testo scuro quando premuto
                            }
                            return const Color(
                                0xFFEC7900); // 🔹 Testo arancione predefinito
                          },
                        ),
                      ),
                      child: const Text(
                        "Scan QR Code",
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
        ],
      ),
    );
  }

  Widget _buildRow(String label, String? value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label,
              style: const TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                  color: Colors.white)),
          Text(value ?? "-",
              style: const TextStyle(fontSize: 18, color: Colors.white)),
        ],
      ),
    );
  }
}

class CornerPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final Paint paint = Paint()
      ..color = Color(0xFFec7900)
      ..strokeWidth = 2
      ..style = PaintingStyle.stroke;
    const double cornerLength = 50.0;
    const double borderRadius = 0;
    canvas.drawLine(Offset(0, borderRadius), Offset(0, cornerLength), paint);
    canvas.drawLine(Offset(borderRadius, 0), Offset(cornerLength, 0), paint);
    canvas.drawLine(Offset(size.width - borderRadius, 0),
        Offset(size.width - cornerLength, 0), paint);
    canvas.drawLine(Offset(size.width, borderRadius),
        Offset(size.width, cornerLength), paint);
    canvas.drawLine(Offset(0, size.height - borderRadius),
        Offset(0, size.height - cornerLength), paint);
    canvas.drawLine(Offset(borderRadius, size.height),
        Offset(cornerLength, size.height), paint);
    canvas.drawLine(Offset(size.width - borderRadius, size.height),
        Offset(size.width - cornerLength, size.height), paint);
    canvas.drawLine(Offset(size.width, size.height - borderRadius),
        Offset(size.width, size.height - cornerLength), paint);
  }

  @override
  bool shouldRepaint(CornerPainter oldDelegate) => false;
}
