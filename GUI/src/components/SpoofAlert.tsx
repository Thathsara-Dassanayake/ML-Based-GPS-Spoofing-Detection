type Props = {
  spoofDetected: boolean;
};

export default function SpoofAlert({ spoofDetected }: Props) {
  if (!spoofDetected) return null;

  return (
    <div className="spoof-banner" role="alert" aria-live="assertive">
      <div className="spoof-title">⚠️ SPOOFING DETECTED</div>
      <div className="spoof-body">
        The ML model has flagged the incoming GNSS data as spoofed. Navigation
        data may be compromised — take appropriate action.
      </div>
    </div>
  );
}
