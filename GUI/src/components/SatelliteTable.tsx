import type { Satellite } from "../App";

type Props = {
  satellites: Satellite[];
};

export default function SatelliteTable({ satellites }: Props) {
  return (
    <section className="sat-table">
      <h2>Satellite Details</h2>

      <table>
        <thead>
          <tr>
            <th>PRN</th>
            <th>SNR (dB-Hz)</th>
            <th>Doppler (Hz)</th>
            <th>Elevation (°)</th>
            <th>Status</th>
          </tr>
        </thead>

        <tbody>
          {satellites.map((s) => (
            <tr key={s.prn} className={s.status.toLowerCase()}>
              <td>{s.prn}</td>
              <td>{s.snr}</td>
              <td>{s.doppler}</td>
              <td>{s.elevation}</td>
              <td>{s.status}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}
