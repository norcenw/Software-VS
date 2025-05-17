import 'package:flutter/material.dart';
import 'dart:convert';
import 'package:http/http.dart' as http;
import '../components/functions.dart'; // 🔹 Importa l'overlay
import '../splash_screen.dart'; // 🔹 Importa per usare `globalServerIp`

class Page1 extends StatefulWidget {
  const Page1({super.key});

  @override
  State<Page1> createState() => _Page1State();
}

class _Page1State extends State<Page1>
    with AutomaticKeepAliveClientMixin<Page1> {
  @override
  bool get wantKeepAlive => true; // ✅ Mantiene lo stato della pagina

  Map<String, dynamic> motorData = {};
  final Map<String, TextEditingController> controllers = {};

  @override
  void initState() {
    super.initState();
    _initializeControllers();
  }

  /// ✅ **Crea un controller per ogni chiave**
  void _initializeControllers() {
    for (var key in [
      "xspeed",
      "yspeed",
      "zspeed",
      "xacc",
      "yacc",
      "zacc",
      "xdelay",
      "ydelay",
      "zdelay",
      "xtime",
      "ytime",
      "ztime",
      "xstep",
      "ystep",
      "zstep"
    ]) {
      if (!controllers.containsKey(key)) {
        controllers[key] = TextEditingController();
      }
    }
  }

  Future<void> fetchMotorData() async {
    String url = "http://$globalServerIp/op=r_1";
    print("📡 Effettuando richiesta a: $url");

    OverlayHelper.showOverlay(context, OverlayType.loading);

    try {
      final response = await http.get(Uri.parse(url));

      if (response.statusCode == 200) {
        String jsonString = response.body;
        print("📝 Risposta ricevuta: $jsonString");

        // 🔹 Decodifica il JSON
        final Map<String, dynamic> jsonData = json.decode(jsonString);

        print("🔍 Chiavi ricevute: ${jsonData.keys}");

        if (jsonData.isNotEmpty) {
          // 🔹 Rimuovi il suffisso "-read" dalle chiavi
          Map<String, dynamic> cleanedData = {};
          jsonData.forEach((key, value) {
            String newKey = key.replaceAll("-read", "");
            cleanedData[newKey] = value;
          });

          setState(() {
            motorData = cleanedData;
          });

          // 🔹 Aggiorna i controller con i nuovi valori
          cleanedData.forEach((key, value) {
            if (controllers.containsKey(key)) {
              controllers[key]?.text = _formatValue(key, value);
            }
          });

          // 🔹 Controlliamo le chiavi Y specificamente
          for (var key in ["yspeed", "yacc", "ydelay", "ytime", "ystep"]) {
            if (!cleanedData.containsKey(key)) {
              print("⚠️ Manca la chiave nel JSON: $key");
            }
          }
        }
      }
    } catch (e) {
      print("⚠️ Errore di connessione: $e");
      OverlayHelper.showOverlay(context, OverlayType.error);
      await Future.delayed(const Duration(seconds: 2));
    } finally {
      OverlayHelper.hideOverlay();
    }
  }

  String _formatValue(String key, dynamic value) {
    double numericValue = double.tryParse(value.toString()) ?? 0;

    if (key == "xdelay" || key == "ydelay" || key == "zdelay") {
      return "${numericValue.toStringAsFixed(3)} ms";
    } else if (key == "xacc" || key == "yacc" || key == "zacc") {
      if (numericValue.toInt() == 0) {
        return "FULL";
      } else if (numericValue.toInt() == 1) {
        return "1/2";
      } else if (numericValue.toInt() == 2) {
        return "1/4";
      } else if (numericValue.toInt() == 3) {
        return "1/8";
      } else if (numericValue.toInt() == 4) {
        return "1/16";
      } else if (numericValue.toInt() == 5) {
        return "1/32";
      }
      return "${numericValue.toInt()} step/min";
    } else if (key == "xspeed" || key == "yspeed" || key == "zspeed") {
      return "${numericValue.toInt()} step/min";
    } else if (key == "xtime" || key == "ytime" || key == "ztime") {
      return "$numericValue ms";
    } else if (key == "xstep" || key == "ystep" || key == "zstep") {
      return "$numericValue mm";
    }
    return value.toString();
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    _initializeControllers(); // ✅ Inizializza i controller

    return Scaffold(
      backgroundColor: Colors.transparent,
      body: SafeArea(
        child: Stack(
          children: [
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const Text(
                  "MOTORS STATUS",
                  style: TextStyle(
                      color: Colors.white,
                      fontSize: 20,
                      fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 20),
                Expanded(
                  child: Padding(
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                    child: ListView(
                      children: [
                        _buildMotorContainer("Motore X", "x"),
                        _buildMotorContainer("Motore Y", "y"),
                        _buildMotorContainer("Motore Z", "z"),
                      ],
                    ),
                  ),
                ),
              ],
            ),
            Positioned(
              bottom: 20,
              right: 20,
              child: Container(
                decoration: BoxDecoration(
                  color: const Color(0xFF0D1D31),
                  borderRadius: BorderRadius.circular(3),
                  border: Border.all(color: const Color(0xFFEC7900), width: 1),
                ),
                child: IconButton(
                  icon: const Icon(Icons.refresh, color: Color(0xFFEC7900)),
                  onPressed: fetchMotorData,
                  padding: const EdgeInsets.all(12),
                  constraints: const BoxConstraints(),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  /// 🔹 Crea un contenitore per ogni motore
  Widget _buildMotorContainer(String title, String axis) {
    return Card(
      color: const Color(0xFF0D1D31),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(3),
        side: const BorderSide(
          color: Color(0xFFEC7900),
          width: 1,
        ),
      ),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              title,
              style: const TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                  color: Colors.white),
            ),
            const SizedBox(height: 10),
            _buildMotorField("Delay", "${axis}delay"),
            _buildMotorField("microStep", "${axis}acc"),
            _buildMotorField("Speed", "${axis}speed"),
            _buildMotorField("Time", "${axis}time"),
            _buildMotorField("Step", "${axis}step"),
          ],
        ),
      ),
    );
  }

  /// 🔹 Crea un campo TextFormField con unità di misura
  Widget _buildMotorField(String label, String key) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 5),
      child: TextFormField(
        key: Key(key), // ✅ Evita problemi di aggiornamento UI
        controller: controllers[key], // ✅ Usa controller invece di initialValue
        readOnly: true,
        style: const TextStyle(
          color: Colors.white,
          fontWeight: FontWeight.bold,
        ),
        decoration: InputDecoration(
          labelText: label,
          labelStyle: const TextStyle(
            color: Color(0xFFEC7900),
            fontWeight: FontWeight.bold,
          ),
          border: const OutlineInputBorder(
            borderSide: BorderSide(
              color: Color(0xFFEC7900),
              width: 1,
            ),
          ),
          enabledBorder: const OutlineInputBorder(
            borderSide: BorderSide(
              color: Color(0xFF0D1D31),
              width: 1,
            ),
          ),
          focusedBorder: const OutlineInputBorder(
            borderSide: BorderSide(
              color: Color(0xFFEC7900),
              width: 1,
            ),
          ),
          filled: true,
          fillColor: Colors.transparent,
        ),
      ),
    );
  }
}
