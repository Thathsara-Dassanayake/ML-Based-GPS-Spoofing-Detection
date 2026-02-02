import type { Satellite } from "../App";

type Props = {
  spoofDetected: boolean;
  satellites: Satellite[];
};

export default function StatusPanel({
  spoofDetected,
  satellites,
}: Props) {
  const tracked = satellites.filter(s => s.status === "TRACKING").length;
  const suspect = satellites.filter(s => s.status === "SUSPECT").length;
  const lost = satellites.filter(s => s.status === "LOST").length;

  return (
    <div className="status-panel">
      <div>
        <strong>Receiver:</strong> RUNNING
      </div>
      <div>
        <strong>Spoofing:</strong>{" "}
        {spoofDetected ? "SPOOFED" : "SAFE"}
      </div>
      <div>
        <strong>Tracked:</strong> {tracked}
      </div>
      <div>
        <strong>Suspect:</strong> {suspect}
      </div>
      <div>
        <strong>Lost:</strong> {lost}
      </div>
    </div>
  );
}
