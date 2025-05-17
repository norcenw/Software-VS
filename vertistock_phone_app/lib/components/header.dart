import 'package:flutter/material.dart';
import '../splash_screen.dart'; // 🔹 Importa per usare `globalServerIp`

class Header extends StatelessWidget {
  final double height;

  const Header({super.key, required this.height});

  @override
  Widget build(BuildContext context) {
    return SafeArea(
      child: Stack(
        children: [
          Container(
            height: height,
            decoration: const BoxDecoration(
              color: Color(0xFF0D1D31), // ✅ Sfondo scuro
              border: Border(
                bottom: BorderSide(
                  color: Color(0xFFEC7900), // ✅ Bordo inferiore arancione
                  width: 1,
                ),
              ),
            ),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.start,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                const SizedBox(width: 10),
                Image.asset('assets/img/logo_vertistock.png', height: 80),
                const SizedBox(width: 10),
                Image.asset('assets/img/dark.png', height: 40),
              ],
            ),
          ),

          // ✅ Positioned per IP (verde)
          Positioned(
            right: 20,
            top: 10,
            child: Text(
              "IP: $globalServerIp",
              style: const TextStyle(
                color: Color(0xFF27A644), // ✅ Verde
                fontSize: 12,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),

          // ✅ Positioned per la versione del file (bianco)
          Positioned(
            right: 20,
            top: 30,
            child: const Text(
              "version: 1.0.0",
              style: TextStyle(
                color: Colors.white, // ✅ Bianco
                fontSize: 12,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
        ],
      ),
    );
  }
}
