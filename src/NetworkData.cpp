#include "NetworkData.h"

#include <string>

// -----------------------------------------------------------------------------
//  Delhi NCR sample transit network.
//
//  60 stations grouped by geographic zone (central/north/south/west/east,
//  Gurgaon, Noida, airport), 124 edges across:
//
//    - 68 metro edges across 8 lines
//        Yellow, Blue, Blue Branch, Violet, Pink, Red, Airport Express, Magenta
//    - 32 bus  edges across 12 DTC-style routes (500..610)
//    - 24 walking transfers at interchanges / adjacent stations
//
//  Times are minutes, fares are INR. Travel times/fares are approximate but
//  internally consistent (~3 min/segment on metro core, ~2-4 min in CP,
//  ~3-4 min for long outer metro hops, ~10-40 min for bus hops, 5-10 min for
//  platform walks). Fares scale roughly with hop count / distance in line
//  with DMRC's slab pricing.
//
//  Station degrees are deliberately weighted so that the real-world high-
//  traffic interchanges (Rajiv Chowk, Kashmere Gate, Mandi House, Central
//  Secretariat, Hauz Khas, Lajpat Nagar, Botanical Garden, Anand Vihar,
//  Karol Bagh) sit on three or more distinct lines / modes and therefore
//  dominate the adjacency list's high-degree tail.
// -----------------------------------------------------------------------------

void loadDelhiNCRNetwork(Graph& g) {
    auto S = [&](const std::string& name) { return g.addStation(name); };

    // =========================================================================
    //  STATION REGISTRY (60 stations)
    //  Registration order is geographic, purely for readability.
    // =========================================================================

    // ---- Zone: Central Delhi / Connaught Place spine (12) ----
    const int rajivChowk      = S("Rajiv Chowk");
    const int newDelhi        = S("New Delhi");
    const int patelChowk      = S("Patel Chowk");
    const int centralSec      = S("Central Secretariat");
    const int udyogBhawan     = S("Udyog Bhawan");
    const int mandiHouse      = S("Mandi House");
    const int chandniChowk    = S("Chandni Chowk");
    const int rkAshramMarg    = S("RK Ashram Marg");
    const int karolBagh       = S("Karol Bagh");
    const int rajendraPlace   = S("Rajendra Place");
    const int patelNagar      = S("Patel Nagar");
    const int shadipur        = S("Shadipur");

    // ---- Zone: North Delhi (7) ----
    const int kashmereGate    = S("Kashmere Gate");
    const int civilLines      = S("Civil Lines");
    const int vishwavidyalaya = S("Vishwavidyalaya");
    const int azadpur         = S("Azadpur");
    const int shastriNagar    = S("Shastri Nagar");
    const int inderlok        = S("Inderlok");
    const int rohini          = S("Rohini");

    // ---- Zone: West Delhi / Janakpuri / Dwarka corridor (10) ----
    const int punjabiBagh     = S("Punjabi Bagh");
    const int rajouriGarden   = S("Rajouri Garden");
    const int kirtiNagar      = S("Kirti Nagar");
    const int motiNagar       = S("Moti Nagar");
    const int tilakNagar      = S("Tilak Nagar");
    const int janakpuriEast   = S("Janakpuri East");
    const int janakpuriWest   = S("Janakpuri West");
    const int dwarkaSec10     = S("Dwarka Sector 10");
    const int dwarka          = S("Dwarka");
    const int dwarkaSec21     = S("Dwarka Sector 21");

    // ---- Zone: South Delhi / Yellow Line spine (12) ----
    const int ina             = S("INA");
    const int aiims           = S("AIIMS");
    const int greenPark       = S("Green Park");
    const int hauzKhas        = S("Hauz Khas");
    const int malviyaNagar    = S("Malviya Nagar");
    const int saket           = S("Saket");
    const int qutubMinar      = S("Qutub Minar");
    const int chhatarpur      = S("Chhatarpur");
    const int sultanpur       = S("Sultanpur");
    const int ghitorni        = S("Ghitorni");
    const int arjanGarh       = S("Arjan Garh");
    const int lajpatNagar     = S("Lajpat Nagar");

    // ---- Zone: Gurgaon (6) ----
    const int guruDronacharya = S("Guru Dronacharya");
    const int sikanderpur     = S("Sikanderpur");
    const int mgRoad          = S("MG Road");
    const int iffcoChowk      = S("IFFCO Chowk");
    const int hudaCityCentre  = S("HUDA City Centre");
    const int gurgaonCyberCity = S("Gurgaon Cyber City");

    // ---- Zone: East Delhi / Ghaziabad (8) ----
    const int yamunaBank      = S("Yamuna Bank");
    const int akshardham      = S("Akshardham");
    const int laxmiNagar      = S("Laxmi Nagar");
    const int preetVihar      = S("Preet Vihar");
    const int karkardooma     = S("Karkardooma");
    const int anandVihar      = S("Anand Vihar");
    const int vaishali        = S("Vaishali");
    const int mayurVihar      = S("Mayur Vihar");

    // ---- Zone: Noida (4) ----
    const int sector18Noida   = S("Sector 18 Noida");
    const int botanicalGarden = S("Botanical Garden");
    const int noidaSec62      = S("Noida Sector 62");
    const int noidaCityCentre = S("Noida City Centre");

    // ---- Airport (1) ----
    const int igiAirport      = S("IGI Airport");

    // =========================================================================
    //  Edge helpers
    // =========================================================================
    auto M = [&](int a, int b, int t, int f, const std::string& line) {
        g.addEdge(a, b, t, f, TransportMode::Metro, line);
    };
    auto B = [&](int a, int b, int t, int f, const std::string& line) {
        g.addEdge(a, b, t, f, TransportMode::Bus, line);
    };
    auto W = [&](int a, int b, int t) {
        g.addEdge(a, b, t, 0, TransportMode::Walking, "Walk");
    };

    // =========================================================================
    //  METRO LINES  (68 edges total)
    // =========================================================================

    // ---- Yellow Line : Azadpur -> HUDA City Centre  (25 edges, north-south) ----
    const std::string YL = "Yellow Line";
    M(azadpur,         vishwavidyalaya, 3, 10, YL);
    M(vishwavidyalaya, civilLines,      3, 10, YL);
    M(civilLines,      kashmereGate,    3, 10, YL);
    M(kashmereGate,    chandniChowk,    3, 10, YL);
    M(chandniChowk,    newDelhi,        3, 10, YL);
    M(newDelhi,        rajivChowk,      2, 10, YL);
    M(rajivChowk,      patelChowk,      2, 10, YL);
    M(patelChowk,      centralSec,      2, 10, YL);
    M(centralSec,      udyogBhawan,     3, 20, YL);
    M(udyogBhawan,     ina,             3, 20, YL);
    M(ina,             aiims,           3, 20, YL);
    M(aiims,           greenPark,       3, 20, YL);
    M(greenPark,       hauzKhas,        3, 20, YL);
    M(hauzKhas,        malviyaNagar,    3, 20, YL);
    M(malviyaNagar,    saket,           3, 30, YL);
    M(saket,           qutubMinar,      3, 30, YL);
    M(qutubMinar,      chhatarpur,      4, 30, YL);
    M(chhatarpur,      sultanpur,       3, 30, YL);
    M(sultanpur,       ghitorni,        3, 40, YL);
    M(ghitorni,        arjanGarh,       3, 40, YL);
    M(arjanGarh,       guruDronacharya, 4, 40, YL);
    M(guruDronacharya, sikanderpur,     3, 50, YL);
    M(sikanderpur,     mgRoad,          3, 50, YL);
    M(mgRoad,          iffcoChowk,      3, 50, YL);
    M(iffcoChowk,      hudaCityCentre,  3, 60, YL);

    // ---- Blue Line (main) : Dwarka Sec 21 -> Noida City Centre  (22 edges) ----
    const std::string BL = "Blue Line";
    M(dwarkaSec21,     dwarka,          3, 10, BL);
    M(dwarka,          dwarkaSec10,     3, 10, BL);
    M(dwarkaSec10,     janakpuriWest,   3, 10, BL);
    M(janakpuriWest,   janakpuriEast,   3, 10, BL);
    M(janakpuriEast,   tilakNagar,      3, 20, BL);
    M(tilakNagar,      rajouriGarden,   3, 20, BL);
    M(rajouriGarden,   kirtiNagar,      3, 20, BL);
    M(kirtiNagar,      motiNagar,       3, 20, BL);
    M(motiNagar,       shadipur,        3, 20, BL);
    M(shadipur,        patelNagar,      2, 20, BL);
    M(patelNagar,      rajendraPlace,   2, 30, BL);
    M(rajendraPlace,   karolBagh,       2, 30, BL);
    M(karolBagh,       rkAshramMarg,    2, 30, BL);
    M(rkAshramMarg,    rajivChowk,      2, 30, BL);
    M(rajivChowk,      mandiHouse,      3, 30, BL);
    M(mandiHouse,      yamunaBank,      4, 30, BL);
    M(yamunaBank,      akshardham,      3, 40, BL);
    M(akshardham,      mayurVihar,      3, 40, BL);
    M(mayurVihar,      sector18Noida,   4, 40, BL);
    M(sector18Noida,   botanicalGarden, 3, 50, BL);
    M(botanicalGarden, noidaSec62,      4, 50, BL);
    M(noidaSec62,      noidaCityCentre, 3, 60, BL);

    // ---- Blue Line Branch : Yamuna Bank -> Vaishali  (5 edges) ----
    const std::string BLB = "Blue Line Branch";
    M(yamunaBank,      laxmiNagar,      3, 20, BLB);
    M(laxmiNagar,      preetVihar,      3, 20, BLB);
    M(preetVihar,      karkardooma,     3, 20, BLB);
    M(karkardooma,     anandVihar,      3, 30, BLB);
    M(anandVihar,      vaishali,        3, 30, BLB);

    // ---- Violet Line : Kashmere Gate -> Lajpat Nagar  (3 edges) ----
    const std::string VL = "Violet Line";
    M(kashmereGate,    mandiHouse,      4, 20, VL);
    M(mandiHouse,      centralSec,      3, 20, VL);
    M(centralSec,      lajpatNagar,     5, 30, VL);

    // ---- Pink Line : ring-style connector  (6 edges) ----
    const std::string PL = "Pink Line";
    M(azadpur,         punjabiBagh,     4, 20, PL);
    M(punjabiBagh,     rajouriGarden,   4, 20, PL);
    M(rajouriGarden,   ina,             6, 30, PL);
    M(ina,             lajpatNagar,     3, 20, PL);
    M(lajpatNagar,     mayurVihar,      5, 30, PL);
    M(mayurVihar,      anandVihar,      4, 30, PL);

    // ---- Red Line : Rohini -> Kashmere Gate  (3 edges) ----
    const std::string RL = "Red Line";
    M(rohini,          inderlok,        4, 20, RL);
    M(inderlok,        shastriNagar,    3, 10, RL);
    M(shastriNagar,    kashmereGate,    4, 20, RL);

    // ---- Airport Express : New Delhi -> IGI -> Dwarka Sec 21  (2 edges) ----
    const std::string AE = "Airport Express";
    M(newDelhi,        igiAirport,      15, 60, AE);
    M(igiAirport,      dwarkaSec21,      8, 40, AE);

    // ---- Magenta Line : Janakpuri West -> Hauz Khas -> Botanical Garden  (2) ----
    const std::string ML = "Magenta Line";
    M(janakpuriWest,   hauzKhas,        12, 40, ML);
    M(hauzKhas,        botanicalGarden, 15, 50, ML);

    // =========================================================================
    //  BUS ROUTES  (32 edges across 12 routes)
    // =========================================================================

    // ---- DTC-500 : Saket - Gurgaon feeder ----
    const std::string B500 = "Bus DTC-500";
    B(saket,            gurgaonCyberCity, 35, 25, B500);
    B(gurgaonCyberCity, mgRoad,           10, 15, B500);
    B(gurgaonCyberCity, iffcoChowk,       12, 15, B500);

    // ---- DTC-510 : Airport / Dwarka feeder ----
    const std::string B510 = "Bus DTC-510";
    B(igiAirport,       gurgaonCyberCity, 25, 25, B510);
    B(dwarka,           igiAirport,       18, 20, B510);

    // ---- DTC-520 : Outer Ring north ----
    const std::string B520 = "Bus DTC-520";
    B(rohini,           azadpur,          15, 15, B520);
    B(rohini,           punjabiBagh,      20, 15, B520);

    // ---- DTC-530 : East cross-link ----
    const std::string B530 = "Bus DTC-530";
    B(noidaSec62,       anandVihar,       22, 20, B530);
    B(mayurVihar,       laxmiNagar,       10, 10, B530);

    // ---- DTC-540 : CP - Airport ----
    const std::string B540 = "Bus DTC-540";
    B(karolBagh,        chandniChowk,     15, 10, B540);
    B(newDelhi,         igiAirport,       40, 30, B540);

    // ---- DTC-550 : South spine cross-link ----
    const std::string B550 = "Bus DTC-550";
    B(saket,            lajpatNagar,      18, 15, B550);
    B(hauzKhas,         aiims,            10, 10, B550);

    // ---- DTC-560 : West / East quick hops ----
    const std::string B560 = "Bus DTC-560";
    B(vaishali,         anandVihar,        8, 10, B560);
    B(punjabiBagh,      karolBagh,        18, 15, B560);

    // ---- DTC-570 : North interchange feeder ----
    const std::string B570 = "Bus DTC-570";
    B(civilLines,       kashmereGate,      8, 10, B570);
    B(centralSec,       mandiHouse,       10, 10, B570);

    // ---- DTC-580 : Noida / East feeder ----
    const std::string B580 = "Bus DTC-580";
    B(noidaCityCentre,  botanicalGarden,  12, 15, B580);
    B(sector18Noida,    mayurVihar,       15, 15, B580);
    B(preetVihar,       anandVihar,       10, 10, B580);

    // ---- DTC-590 : Central connector ----
    const std::string B590 = "Bus DTC-590";
    B(rajivChowk,       mandiHouse,        8, 10, B590);
    B(punjabiBagh,      azadpur,          15, 15, B590);
    B(anandVihar,       karkardooma,       8, 10, B590);

    // ---- DTC-600 : South / East cross ----
    const std::string B600 = "Bus DTC-600";
    B(lajpatNagar,      centralSec,       14, 15, B600);
    B(noidaSec62,       vaishali,         25, 20, B600);
    B(rajouriGarden,    punjabiBagh,      10, 10, B600);

    // ---- DTC-610 : Various short feeders ----
    const std::string B610 = "Bus DTC-610";
    B(janakpuriWest,    dwarkaSec10,      12, 10, B610);
    B(chhatarpur,       qutubMinar,       10, 10, B610);
    B(arjanGarh,        sultanpur,        10, 10, B610);
    B(gurgaonCyberCity, sikanderpur,       8, 10, B610);
    B(noidaCityCentre,  noidaSec62,        8, 10, B610);
    B(karolBagh,        patelNagar,        8, 10, B610);

    // =========================================================================
    //  WALKING TRANSFERS  (24 edges)
    //  Used as free platform-level connectors where real DMRC interchanges or
    //  adjacent stations are within reasonable walking distance.
    // =========================================================================
    W(newDelhi,         rajivChowk,        5);
    W(mandiHouse,       rajivChowk,        7);
    W(patelChowk,       rajivChowk,        5);
    W(centralSec,       patelChowk,        6);
    W(kashmereGate,     civilLines,        8);
    W(civilLines,       vishwavidyalaya,   7);
    W(karolBagh,        rajendraPlace,     6);
    W(rajendraPlace,    patelNagar,        6); // chain continuation on CP spine
    W(patelNagar,       shadipur,          6);
    W(motiNagar,        kirtiNagar,        6);
    W(aiims,            ina,               5);
    W(hauzKhas,         greenPark,         6);
    W(saket,            malviyaNagar,      7);
    W(sikanderpur,      gurgaonCyberCity, 10);
    W(mgRoad,           iffcoChowk,        8);
    W(iffcoChowk,       hudaCityCentre,    8);
    W(ghitorni,         arjanGarh,         7);
    W(botanicalGarden,  sector18Noida,     7);
    W(yamunaBank,       akshardham,        6);
    W(lajpatNagar,      ina,               8);
    W(janakpuriEast,    janakpuriWest,     6);
    W(shastriNagar,     inderlok,          6);
    W(noidaSec62,       noidaCityCentre,   8);
    W(anandVihar,       vaishali,         10);
}
