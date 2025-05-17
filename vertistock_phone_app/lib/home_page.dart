import 'package:flutter/material.dart';
import 'components/header.dart';
import 'components/footer.dart';
import 'pages/page1.dart';
import 'pages/page2.dart';
import 'pages/page3.dart';
import 'components/functions.dart';

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  int _selectedIndex = 1; // Avvia dalla seconda pagina
  final PageController _pageController = PageController(initialPage: 1);

  void _onItemTapped(int index) {
    changePage(index, _pageController, setState);
  }

  @override
  Widget build(BuildContext context) {
    double screenHeight = MediaQuery.of(context).size.height;
    double statusBarHeight = MediaQuery.of(context).padding.top;
    double headerHeight = 60.0; // Altezza fissa per l'header
    double footerHeight = 60.0; // Altezza fissa per la navbar
    double pageHeight =
        screenHeight - headerHeight - footerHeight - statusBarHeight;

    return Scaffold(
      body: Container(
        decoration: const BoxDecoration(
          gradient: RadialGradient(
            center: Alignment.center,
            radius: 1.0,
            colors: [
              Color(0xFF0D1D31), // #0d1d31
              Color(0xFF0C0D13), // #0c0d13
            ],
          ),
        ),
        child: Column(
          children: [
            Header(height: headerHeight),
            Expanded(
              child: SizedBox(
                height: pageHeight,
                child: PageView(
                  controller: _pageController,
                  onPageChanged: (index) {
                    setState(() {
                      _selectedIndex = index;
                    });
                  },
                  children: [
                    const Page1(),
                    Page2(),
                    const Page3(),
                  ],
                ),
              ),
            ),
            Footer(
              height: footerHeight,
              selectedIndex: _selectedIndex,
              onItemTapped: _onItemTapped,
            ),
          ],
        ),
      ),
    );
  }
}
