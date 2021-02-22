import ROOT

f = open('hist_124.txt')
steps, low, high = list(map(int, f.readline().split()))
entries = list(map(int, f.readline().split()))
f.close()

h = ROOT.TH1F('hist', 'hist', steps, low, high)
for i in range(1, len(entries)+1):
    h.SetBinContent(i, entries[i-1])
h.Draw()
input('Press enter to finish')
