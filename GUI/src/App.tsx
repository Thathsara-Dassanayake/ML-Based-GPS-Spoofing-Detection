import { useState } from "react";
import Header from "./components/Header";
import StatusPanel from "./components/StatusPanel";
import SpoofAlert from "./components/SpoofAlert";
import SatelliteTable from "./components/SatelliteTable";
import "./App.css";

export type Satellite = {
  prn: number;
  snr: number;
  doppler: number;
  elevation: number;
  status: "TRACKING" | "LOST" | "SUSPECT";
};

const initialSats: Satellite[] = [
  { prn: 3, snr: 42, doppler: -2100, elevation: 56, status: "TRACKING" },
  { prn: 7, snr: 18, doppler: 500, elevation: 12, status: "SUSPECT" },
  { prn: 12, snr: 35, doppler: -1800, elevation: 44, status: "TRACKING" },
  { prn: 19, snr: 10, doppler: 0, elevation: 5, status: "LOST" },
];

function App() {
  const [spoofDetected, setSpoofDetected] = useState(false);
  const [satellites] = useState(initialSats);

  return (
    <div className="app-root">
      <Header />

      <div className="main-grid">
        <StatusPanel
          spoofDetected={spoofDetected}
          satellites={satellites}
        />

        <div className="controls">
          <button
            className="demo-btn"
            onClick={() => setSpoofDetected((prev) => !prev)}
          >
            Toggle Spoofing (demo)
          </button>
        </div>
      </div>

      <SpoofAlert spoofDetected={spoofDetected} />
      <SatelliteTable satellites={satellites} />
    </div>
  );
}

export default App;
