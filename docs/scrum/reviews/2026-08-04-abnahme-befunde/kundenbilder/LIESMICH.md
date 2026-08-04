# Kundenbilder der Sprint-6-Abnahme

Aufgenommen vom Kunden am 04.08.2026 am **installierten Stand**
(`/usr/bin/denkzetteld`, Stand `977e804`, Dienst gestartet 16:19).

| Datei | Zeigt | Befund |
|---|---|---|
| `01-denkzettel-neben-dolphin.png` | Erfassungsfenster neben Dolphin unter dem Theme des Kunden | B3 (Farben), B2 (nativer Schattennachbar im selben Bild) |
| `02-win11-dark-theme.png` | dasselbe unter einem Win11-Dark-Theme — *„scheint es zu passen"* | Gegenprobe zu B3 |
| `03-ecke-denkzettel.png` | linke obere Ecke des Erfassungsfensters, 31×32 | B1 |
| `04-ecke-nativ.png` | linke obere Ecke eines nativen Fensters, 49×37 | B1, Vergleichsstück |

## Herkunft, geprüft statt behauptet

Alle vier sind **byteweise gleich** mit den Originalen des Kunden unter
`~/Bilder/Bildschirmfotos/Bildschirmfoto_20260804_<HHMMSS>.png`
(`162432`, `162528`, `163236`, `163723`), geprüft mit `sha256sum`.

**Warum das hier steht:** Zwei der vier lagen zuerst in temporären
Spectacle-Ordnern. Beim Sichern war `Bildschirmfoto_20260804_163236.png` dort
**bereits gelöscht** — der Ordner war fort, wenige Minuten nach der Aufnahme.
Gerettet wurde es aus einem Sitzungs-Zwischenspeicher; diese Fassung wich
byteweise ab (Neukodierung durch den Zwischenspeicher) und ist inzwischen durch
das Original ersetzt.

Damit ist B14 (Retro Sprint 3: *„Flüchtige Belege werden beim Eintreffen
gesichert"*) an genau der Stelle wieder eingetreten, für die er geschrieben
wurde — der Sprint-3-Abnahme. Der Unterschied zu damals ist, dass diesmal
nichts verloren ging.

## Was diese Bilder nicht sind

Sie sind **Kundenbefund, nicht Messung**. Was daraus an Befunden folgt, steht
im Untersuchungsbericht dieses Ordners; was sich davon bestätigt hat und was
nicht, ebenfalls dort.
