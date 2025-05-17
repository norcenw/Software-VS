import 'package:flutter/material.dart';
import 'dart:async';
import 'package:multicast_dns/multicast_dns.dart';
import 'dart:developer';
import 'home_page.dart';

String? globalServerIp; // 🌍 Variabile globale accessibile ovunque

class SplashScreen extends StatefulWidget {
  const SplashScreen({super.key});

  @override
  State<SplashScreen> createState() => _SplashScreenState();
}

class _SplashScreenState extends State<SplashScreen>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _animation;
  String _status = '🔍 Searching IP Address...';
  bool _color = true;
  bool _isSearching = true; // 🔄 Mantiene il loop attivo finché non trova l'IP

  @override
  void initState() {
    super.initState();

    // 🔹 Crea un'animazione infinita di rimpicciolimento e ingrandimento
    _controller = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 2),
      lowerBound: 0.5,
      upperBound: 1.0,
    )..repeat(reverse: true); // 🔄 Ripete all'infinito

    _animation = CurvedAnimation(parent: _controller, curve: Curves.easeInOut);

    _findServerIp(); // 🚀 Avvia la ricerca dell'IP
  }

  /// 🔎 **Cerca l'IP di vertistock tramite mDNS in loop**
  Future<void> _findServerIp() async {
    while (_isSearching && globalServerIp == null) {
      final MDnsClient client = MDnsClient();
      try {
        await client.start();
        log('📡 Avvio scansione di tutti i servizi in _tcp._tcp');

        await for (final PtrResourceRecord ptr
            in client.lookup<PtrResourceRecord>(
          ResourceRecordQuery.serverPointer('_tcp._tcp'),
        )) {
          log('🔍 Servizio trovato: ${ptr.domainName}');

          if (ptr.domainName.contains('_http')) {
            log('🟢 Servizio HTTP rilevato: ${ptr.domainName}');
            await _resolveService(client, ptr.domainName);
          }
        }

        if (globalServerIp == null) {
          await Future.delayed(
              const Duration(seconds: 2)); // 🔄 Aspetta prima di riprovare
        }
      } catch (e) {
        log('❌ Errore mDNS: $e');
      } finally {
        client.stop();
      }
    }
  }

  /// 🌍 **Trova l'IP di vertistock tramite il servizio mDNS**
  Future<void> _resolveService(MDnsClient client, String serviceName) async {
    try {
      await for (final SrvResourceRecord srv
          in client.lookup<SrvResourceRecord>(
        ResourceRecordQuery.service(serviceName),
      )) {
        log('🌍 Server trovato: ${srv.target}:${srv.port}');

        await for (final IPAddressResourceRecord ip
            in client.lookup<IPAddressResourceRecord>(
          ResourceRecordQuery.addressIPv4(srv.target),
        )) {
          log('✅ IP risolto: ${ip.address.address}');
          if (mounted) {
            setState(() {
              globalServerIp = ip.address.address;
              _status = '✅ IP Address: $globalServerIp';
              _color = false;
              _isSearching = false; // ✅ Interrompe il loop
            });
          }

          // ✅ Passa automaticamente a HomePage dopo 1 secondo
          Future.delayed(const Duration(seconds: 1), () {
            if (mounted) {
              Navigator.of(context).pushReplacement(
                MaterialPageRoute(builder: (context) => const HomePage()),
              );
            }
          });

          return;
        }
      }
    } catch (e) {
      log('❌ Errore nella risoluzione di $serviceName: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        width: double.infinity, // ✅ Copre tutto lo schermo
        height: double.infinity, // ✅ Evita bande laterali
        decoration: const BoxDecoration(
          gradient: RadialGradient(
            center: Alignment.center,
            radius: 1.0,
            colors: [
              Color(0xFF0D1D31),
              Color(0xFF0C0D13),
            ],
          ),
        ),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            ScaleTransition(
              scale: _animation,
              child: Image.asset('assets/img/logo_vertistock.png', height: 300),
            ),
            const SizedBox(height: 20),
            Text(
              _status,
              style: TextStyle(
                color: _color
                    ? const Color(0xFF096cd5)
                    : const Color(0xFF27a644), // ✅ Fixato
                fontSize: 18,
              ),
            ),
          ],
        ),
      ),
    );
  }

  @override
  void dispose() {
    if (_controller.isAnimating) {
      _controller.stop(); // ✅ Ferma l'animazione prima della dispose
    }
    _controller.dispose();
    super.dispose();
  }
}
