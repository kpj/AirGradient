import datetime

import pandas as pd

import seaborn as sns


def main(fname_data):
    df = pd.read_csv(fname_data)
    df["timestamp"] = df["timestamp"].apply(
        lambda ts: datetime.datetime.fromtimestamp(ts)
    )
    print(df.head())

    g = sns.FacetGrid(
        data=df.melt(id_vars=["timestamp"]),
        col="variable",
        col_wrap=3,
        sharey=False,
        height=5,
    )
    g.map_dataframe(sns.lineplot, x="timestamp", y="value")
    g.fig.autofmt_xdate()
    g.savefig("measures.pdf")


if __name__ == "__main__":
    main("data.csv")
