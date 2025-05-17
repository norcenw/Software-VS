import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';

class Footer extends StatelessWidget {
  final double height;
  final int selectedIndex;
  final Function(int) onItemTapped;

  const Footer(
      {super.key,
      required this.height,
      required this.selectedIndex,
      required this.onItemTapped});

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      height: height,
      child: ClipRRect(
        child: Container(
          decoration: const BoxDecoration(
            border: Border(
              top: BorderSide(
                color: Color(0xFFEC7900), // ✅ Bordo superiore arancione
                width: 1, // ✅ Spessore di 1px
              ),
            ),
          ),
          child: BottomNavigationBar(
            backgroundColor: const Color(0xFF0D1D31), // ✅ Sfondo scuro
            elevation: 0, // ✅ Rimuove ombra
            currentIndex: selectedIndex,
            onTap: onItemTapped,
            selectedItemColor: Colors.white,
            unselectedItemColor: Colors.grey,
            items: [
              BottomNavigationBarItem(
                icon: SvgPicture.asset('assets/img/view.svg', height: 20),
                label: 'View',
              ),
              BottomNavigationBarItem(
                icon: SvgPicture.asset('assets/img/warehouse.svg', height: 20),
                label: 'Manage',
              ),
              BottomNavigationBarItem(
                icon: SvgPicture.asset('assets/img/qrcode.svg', height: 20),
                label: 'Scan',
              ),
            ],
          ),
        ),
      ),
    );
  }
}
