import 'package:flutter/material.dart';
import 'dart:math';

import 'package:lottie/lottie.dart';

void changePage(int index, PageController controller, Function setState) {
  setState(() {
    controller.animateToPage(index,
        duration: const Duration(milliseconds: 300), curve: Curves.easeInOut);
  });
}

enum OverlayType { loading, error, success, warning }

class OverlayHelper {
  static OverlayEntry? _overlayEntry;

  /// 🔹 **Mostra l'overlay con animazione**
  static void showOverlay(BuildContext context, OverlayType type) {
    if (_overlayEntry != null) return; // ✅ Evita overlay multipli

    Widget overlayContent;

    if (type == OverlayType.loading) {
      overlayContent = const LoadingAnimation(); // 🔄 Usa animazione Flutter
    } else {
      overlayContent = _getLottieAnimation(type);
    }

    _overlayEntry = OverlayEntry(
      builder: (context) => Stack(
        children: [
          Positioned.fill(
            child: Container(color: Colors.black.withOpacity(0.6)),
          ),
          Center(child: overlayContent),
        ],
      ),
    );

    WidgetsBinding.instance.addPostFrameCallback((_) {
      Overlay.of(context).insert(_overlayEntry!);
    });
  }

  /// 🔹 **Nasconde manualmente l'overlay**
  static void hideOverlay() {
    _overlayEntry?.remove();
    _overlayEntry = null;
  }

  /// 🔹 **Restituisce l'animazione Lottie**
  static Widget _getLottieAnimation(OverlayType type) {
    String animationPath = _getAnimationPath(type);
    return Lottie.asset(
      animationPath,
      width: 250,
      height: 250,
      fit: BoxFit.contain,
    );
  }

  /// 🔹 **Seleziona il file Lottie per errori, success, warning**
  static String _getAnimationPath(OverlayType type) {
    switch (type) {
      case OverlayType.error:
        return 'assets/lottie/fail.json';
      case OverlayType.success:
        return 'assets/lottie/correct.json';
      case OverlayType.warning:
        return 'assets/lottie/warning.json';
      default:
        return 'assets/lottie/loading.json'; // Non viene mai usato ora
    }
  }
}

/// 🔄 **Animazione di caricamento personalizzata (Flutter)**
class LoadingAnimation extends StatefulWidget {
  const LoadingAnimation({super.key});

  @override
  _LoadingAnimationState createState() => _LoadingAnimationState();
}

class _LoadingAnimationState extends State<LoadingAnimation>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 2),
    )..repeat();
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 100,
      height: 100,
      child: AnimatedBuilder(
        animation: _controller,
        builder: (context, child) {
          return CustomPaint(
            painter: _LoadingPainter(_controller.value),
          );
        },
      ),
    );
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }
}

class _LoadingPainter extends CustomPainter {
  final double progress;
  _LoadingPainter(this.progress);

  @override
  void paint(Canvas canvas, Size size) {
    final Paint paint = Paint()
      ..color = const Color(0xFF096CD5)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 8.0
      ..strokeCap = StrokeCap.round;

    final double radius = size.width / 2;
    final Offset center = Offset(size.width / 2, size.height / 2);

    final double startAngle = 2 * pi * progress;
    final double sweepAngle = pi * 1.5;

    canvas.drawArc(
      Rect.fromCircle(center: center, radius: radius),
      startAngle,
      sweepAngle,
      false,
      paint,
    );
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => true;
}
