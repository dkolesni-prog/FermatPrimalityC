import os
import pandas as pd
import matplotlib.pyplot as plt


def main():
    """Analyse Fermat test results and visualise performance metrics.

    This script now ALSO plots:
    - The false-positive rate vs bit-length.
    - The average elapsed time vs bit-length.
    - A log–log scatter of elapsed time vs n (cardinality view).

    All figures go into ``figures/``, and raw CSVs into the same folder.
    """

    # Ensure output folders exist ------------------------------------------------
    os.makedirs("figures", exist_ok=True)

    # ---------------------------------------------------------------------------
    # 1. Load data.
    # ---------------------------------------------------------------------------
    path = os.path.join("data", "results.csv")
    df = pd.read_csv(path)

    # ---------------------------------------------------------------------------
    # 2. Overall false-positive rate
    # ---------------------------------------------------------------------------
    composites = df[df["is_really_prime"] == 0]
    fp_mask = composites["is_probably_prime"] == 1
    fp_rate_overall = fp_mask.mean()
    print(f"Overall false-positive rate: {fp_rate_overall:.6%}")

    # ---------------------------------------------------------------------------
    # 3. Add bit-length column.
    # ---------------------------------------------------------------------------
    df["bit_len"] = df["n"].apply(lambda x: int(x).bit_length())

    # ---------------------------------------------------------------------------
    # 4. False-positive rate by bit-length.
    # ---------------------------------------------------------------------------
    def fp_rate(group: pd.DataFrame) -> float:
        if len(group) == 0:
            return float("nan")
        return (group["is_probably_prime"] == 1).mean()

    fp_rate_by_bits = composites.groupby("bit_len").apply(fp_rate)
    fp_rate_by_bits.index.name = "bit_len"
    fp_rate_by_bits.name = "fp_rate"

    # Store raw numbers
    fp_rate_by_bits.to_csv("figures/fp_rate_vs_bits.csv", header=True)

    # Plot FPR
    plt.figure()
    fp_rate_by_bits.plot(marker="o")
    plt.title("False-positive rate of Fermat test vs bit length")
    plt.xlabel("Bit length (bits)")
    plt.ylabel("False-positive rate")
    plt.yscale("log")
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.tight_layout()
    plt.savefig("figures/fp_rate_vs_bits.png")
    plt.close()

    # ---------------------------------------------------------------------------
    # 5. Nice console read-out of FPR
    # ---------------------------------------------------------------------------
    pd.set_option("display.float_format", lambda v: f"{v:.6%}")
    print("\nFalse-positive rate by bit length:\n")
    print(fp_rate_by_bits.to_string())

    # ---------------------------------------------------------------------------
    # 6. Average elapsed time vs bit-length
    # ---------------------------------------------------------------------------
    avg_time_by_bits = df.groupby("bit_len")["elapsed_ns"].mean()
    avg_time_by_bits.name = "avg_elapsed_ns"
    avg_time_by_bits.to_csv("figures/avg_time_vs_bits.csv", header=True)

    plt.figure()
    avg_time_by_bits.plot(marker="o")
    plt.title("Average elapsed time of Fermat test vs bit length")
    plt.xlabel("Bit length (bits)")
    plt.ylabel("Average elapsed_ns")
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()
    plt.savefig("figures/avg_time_vs_bits.png")
    plt.close()

    print("\nAverage elapsed time by bit length:\n")
    print(avg_time_by_bits.to_string())

    # ---------------------------------------------------------------------------
    # 7. Elapsed time vs n (log–log scatter)
    # ---------------------------------------------------------------------------
    plt.figure(figsize=(8, 6))
    plt.scatter(df["n"], df["elapsed_ns"], s=1, alpha=0.3)
    plt.xscale("log")
    plt.yscale("log")
    plt.title("Elapsed time of Fermat test vs n (log–log)")
    plt.xlabel("n")
    plt.ylabel("elapsed_ns")
    plt.grid(True, which="both", linestyle="--", alpha=0.3)
    plt.tight_layout()
    plt.savefig("figures/time_vs_n_loglog.png")
    plt.close()

    print("\nScatter plot of elapsed time vs n saved to figures/time_vs_n_loglog.png")


if __name__ == "__main__":
    main()
