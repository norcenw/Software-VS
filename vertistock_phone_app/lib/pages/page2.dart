import 'package:flutter/material.dart';
import 'dart:convert';
import 'package:http/http.dart' as http;
import '../home_page.dart';
import '../components/functions.dart';
import '../splash_screen.dart';

class Page2 extends StatefulWidget {
  const Page2({super.key});

  @override
  State<Page2> createState() => _Page2State();
}

class _Page2State extends State<Page2>
    with AutomaticKeepAliveClientMixin<Page2> {
  List<Map<String, dynamic>> warehouseData = [];
  Map<Map<String, dynamic>, int> itemIndices = {};

  @override
  bool get wantKeepAlive => true;

  void initState() {
    super.initState();
  }

  Future<void> fetchWarehouseData1() async {
    String url = "http://$globalServerIp/op=r_3_1";
    print("📡 Effettuando richiesta a: $url");

    OverlayHelper.showOverlay(context, OverlayType.loading);

    try {
      final response = await http.get(Uri.parse(url));

      if (response.statusCode == 200) {
        final Map<String, dynamic> jsonData = json.decode(response.body);
        print("📝 JSON ricevuto: $jsonData");

        List<Map<String, dynamic>> filteredData = [];
        Map<Map<String, dynamic>, int> indices = {};

        for (var key in ["wh1", "wh2"]) {
          if (jsonData.containsKey(key) && jsonData[key] is List) {
            for (int i = 0; i < jsonData[key].length; i++) {
              var item = jsonData[key][i];
              if (item["full"] == 1) {
                filteredData.add(item);
                indices[item] = i;
              }
            }
          }
        }

        setState(() {
          warehouseData.clear(); // ✅ Cancella solo all'inizio
          warehouseData.addAll(filteredData); // ✅ Aggiunge invece di sostituire
          itemIndices.addAll(indices);
        });

        await Future.delayed(const Duration(milliseconds: 500));
        OverlayHelper.hideOverlay();
      }
    } catch (e) {
      if (mounted) {
        OverlayHelper.showOverlay(context, OverlayType.error);
      }
      await Future.delayed(const Duration(seconds: 2));
      OverlayHelper.hideOverlay();
    }
  }

  Future<void> fetchWarehouseData2() async {
    String url = "http://$globalServerIp/op=r_3_2";
    print("📡 Effettuando richiesta a: $url");

    OverlayHelper.showOverlay(context, OverlayType.loading);

    try {
      final response = await http.get(Uri.parse(url));

      if (response.statusCode == 200) {
        final Map<String, dynamic> jsonData = json.decode(response.body);
        print("📝 JSON ricevuto: $jsonData");

        List<Map<String, dynamic>> filteredData = [];
        Map<Map<String, dynamic>, int> indices = {};

        for (var key in ["wh1", "wh2"]) {
          if (jsonData.containsKey(key) && jsonData[key] is List) {
            for (int i = 0; i < jsonData[key].length; i++) {
              var item = jsonData[key][i];
              if (item["full"] == 1) {
                filteredData.add(item);
                indices[item] = i;
              }
            }
          }
        }

        setState(() {
          warehouseData.addAll(filteredData); // ✅ Aggiunge invece di sostituire
          itemIndices.addAll(indices);
        });

        await Future.delayed(const Duration(milliseconds: 500));
        OverlayHelper.hideOverlay();
      }
    } catch (e) {
      if (mounted) {
        OverlayHelper.showOverlay(context, OverlayType.error);
      }
      await Future.delayed(const Duration(seconds: 2));
      OverlayHelper.hideOverlay();
    }
  }

  Future<void> withdrawItem(
      BuildContext context, int itemNumber, int warehouse) async {
    String url = "http://$globalServerIp/op=w_3";
    print("📤 Inviando richiesta a: $url con box=$itemNumber e z=$warehouse");

    OverlayHelper.showOverlay(context, OverlayType.loading);

    try {
      final response = await http.post(
        Uri.parse(url),
        headers: {"Content-Type": "application/json"},
        body: jsonEncode({"box": itemNumber, "z": warehouse}),
      );

      if (response.statusCode == 200) {
        print("✅ Prelievo riuscito!");
        OverlayHelper.showOverlay(context, OverlayType.success);
        await Future.delayed(const Duration(seconds: 2));
        OverlayHelper.hideOverlay();

        Navigator.pushReplacement(
          context,
          MaterialPageRoute(builder: (context) => const HomePage()),
        );
      } else {
        print("❌ Errore prelievo: ${response.statusCode}");
        OverlayHelper.showOverlay(context, OverlayType.error);
        await Future.delayed(const Duration(seconds: 2));
        OverlayHelper.hideOverlay();
      }
    } catch (e) {
      print("❌ Errore connessione: $e");
      OverlayHelper.showOverlay(context, OverlayType.error);
      await Future.delayed(const Duration(seconds: 2));
      OverlayHelper.hideOverlay();
    }
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Scaffold(
      backgroundColor: Colors.transparent,
      body: SafeArea(
        child: Stack(
          children: [
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text(
                  "WAREHOUSE",
                  style: const TextStyle(
                      color: Colors.white,
                      fontSize: 20,
                      fontWeight: FontWeight.bold),
                ),
                const SizedBox(height: 20),
                Expanded(
                  child: Padding(
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                    child: ListView.builder(
                      padding: const EdgeInsets.all(8),
                      itemCount: warehouseData.length,
                      itemBuilder: (context, index) {
                        final item = warehouseData[index];
                        final int itemNumber = itemIndices[item] ?? 0;
                        return Card(
                          color: const Color(0xFF0D1D31),
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(3),
                            side: const BorderSide(
                              color: Color(0xFFEC7900),
                              width: 1,
                            ),
                          ),
                          child: Theme(
                            data: Theme.of(context).copyWith(
                              dividerColor: Colors.transparent,
                              splashColor: Colors.transparent,
                              highlightColor: Colors.transparent,
                            ),
                            child: ExpansionTile(
                              tilePadding:
                                  const EdgeInsets.symmetric(horizontal: 15),
                              childrenPadding: const EdgeInsets.symmetric(
                                  horizontal: 5, vertical: 5),
                              iconColor: const Color(0xFFEC7900),
                              collapsedIconColor: const Color(0xFFEC7900),
                              title: Text(
                                "Name: ${item["name"]}",
                                style: const TextStyle(
                                  fontSize: 18,
                                  fontWeight: FontWeight.bold,
                                  color: Colors.white, // 🔹 Fisso su nero
                                ),
                              ),
                              children: [
                                _buildDetailField("Code:", item["code"]),
                                _buildDetailField("Count:", item["count"]),
                                _buildDetailField("Warehouse:", item["z"]),
                                _buildDetailField("Item Number:", itemNumber),

                                // 🔹 Bottone "Withdraw"
                                Center(
                                  child: Padding(
                                    padding: const EdgeInsets.only(top: 10),
                                    child: ElevatedButton(
                                      onPressed: () {
                                        withdrawItem(
                                            context, itemNumber, item["z"]);
                                      },
                                      style: ElevatedButton.styleFrom(
                                        backgroundColor: const Color(
                                            0xFF0D1D31), // 🔹 Sfondo scuro
                                        padding: const EdgeInsets.symmetric(
                                            horizontal: 50, vertical: 12),
                                        shape: RoundedRectangleBorder(
                                          borderRadius:
                                              BorderRadius.circular(3),
                                          side: const BorderSide(
                                            color: Color(
                                                0xFFEC7900), // 🔸 Bordo arancione 1px
                                            width: 1,
                                          ),
                                        ),
                                      ).copyWith(
                                        backgroundColor: MaterialStateProperty
                                            .resolveWith<Color>(
                                          (Set<MaterialState> states) {
                                            if (states.contains(
                                                MaterialState.pressed)) {
                                              return const Color(
                                                  0xFFEC7900); // 🔹 Cambia sfondo quando premuto
                                            }
                                            return const Color(
                                                0xFF0D1D31); // 🔹 Colore predefinito
                                          },
                                        ),
                                        foregroundColor: MaterialStateProperty
                                            .resolveWith<Color>(
                                          (Set<MaterialState> states) {
                                            if (states.contains(
                                                MaterialState.pressed)) {
                                              return const Color(
                                                  0xFF0D1D31); // 🔹 Cambia testo quando premuto
                                            }
                                            return const Color(
                                                0xFFEC7900); // 🔹 Colore predefinito
                                          },
                                        ),
                                      ),
                                      child: const Text(
                                        "Withdraw",
                                        style: TextStyle(
                                            fontSize: 16,
                                            fontWeight: FontWeight.bold),
                                      ),
                                    ),
                                  ),
                                ),
                              ],
                            ),
                          ),
                        );
                      },
                    ),
                  ),
                ),
              ],
            ),

            // 🔹 Bottone Floating Reload (in basso a destra)
            Positioned(
              bottom: 20,
              right: 20,
              child: Container(
                decoration: BoxDecoration(
                  color: const Color(0xFF0D1D31),
                  borderRadius:
                      BorderRadius.circular(3), // ✅ Bordo arrotondato 3px
                  border: Border.all(
                      color: const Color(0xFFEC7900),
                      width: 1), // ✅ Bordo arancione 1px
                ),
                child: IconButton(
                  icon: const Icon(Icons.refresh,
                      color: Color(0xFFEC7900)), // ✅ Icona arancione
                  onPressed: () async {
                    setState(() {
                      warehouseData.clear(); // ✅ Cancella i dati attuali
                      itemIndices.clear(); // ✅ Cancella gli indici
                    });

                    await fetchWarehouseData1();
                    await fetchWarehouseData2();
                  },
                  padding:
                      const EdgeInsets.all(12), // ✅ Spazio intorno all'icona
                  constraints:
                      const BoxConstraints(), // ✅ Rimuove padding extra
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  /// 🔹 Crea un campo TextFormField non modificabile senza cambi di colore al tocco
  Widget _buildDetailField(String label, dynamic value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 5),
      child: TextFormField(
        initialValue: value.toString(),
        readOnly: true,
        style: const TextStyle(
          color: Colors.white, // 🔹 Testo sempre bianco
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
              color: Color(0xFFEC7900), // 🔸 Bordo arancione
              width: 1,
            ),
          ),
          enabledBorder: const OutlineInputBorder(
            borderSide: BorderSide(
              color: Color(
                  0xFF0D1D31), // 🔸 Bordo arancione anche quando non è selezionato
              width: 1,
            ),
          ),
          focusedBorder: const OutlineInputBorder(
            borderSide: BorderSide(
              color: Color(
                  0xFFEC7900), // 🔸 Bordo arancione anche quando è selezionato
              width: 1,
            ),
          ),
          filled: true,
          fillColor: Colors.transparent, // 🔹 Sfondo trasparente
        ),
      ),
    );
  }
}
