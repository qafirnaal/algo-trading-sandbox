export default function ConditionRow({
  condition,
  onChange,
  onRemove,
  signals
}) {
  const update = (patch) => {
    onChange({ ...condition, ...patch });
  };

  return (
    <div className="condition-row">
      <select
        value={condition.lhs}
        onChange={(e) => update({ lhs: e.target.value })}
      >
        {signals.map(s => (
          <option key={s} value={s}>{s}</option>
        ))}
      </select>

      <select
        value={condition.op}
        onChange={(e) => update({ op: e.target.value })}
      >
        <option value="<">&lt;</option>
        <option value=">">&gt;</option>
      </select>

      <select
        value={condition.rhs_type}
        onChange={(e) =>
          update({
            rhs_type: e.target.value,
            rhs_value: e.target.value === "CONSTANT" ? 0 : undefined,
            rhs_signal: e.target.value === "SIGNAL" ? signals[0] : undefined
          })
        }
      >
        <option value="CONSTANT">Value</option>
        <option value="SIGNAL">Signal</option>
      </select>

      {condition.rhs_type === "CONSTANT" ? (
        <input
          type="number"
          value={condition.rhs_value}
          onChange={(e) =>
            update({ rhs_value: Number(e.target.value) })
          }
        />
      ) : (
        <select
          value={condition.rhs_signal}
          onChange={(e) =>
            update({ rhs_signal: e.target.value })
          }
        >
          {signals.map(s => (
            <option key={s} value={s}>{s}</option>
          ))}
        </select>
      )}

      <button
        className="remove-btn"
        onClick={onRemove}
        aria-label="Remove condition"
      >
        ×
      </button>
    </div>
  );
}
