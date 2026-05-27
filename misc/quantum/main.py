import matplotlib.pyplot as plt
from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator

sim = AerSimulator()

shots = 1024
runs_list = [10, 100, 1000]

p0_list = []
p1_list = []

for runs in runs_list:
    total_0 = 0
    total_1 = 0

    for _ in range(runs):
        qc = QuantumCircuit(1, 1)
        qc.h(0)
        qc.measure(0, 0)

        result = sim.run(qc, shots=shots).result()
        counts = result.get_counts()

        total_0 += counts.get('0', 0)
        total_1 += counts.get('1', 0)

    total_shots = runs * shots

    p0 = (total_0 / total_shots) * 100
    p1 = (total_1 / total_shots) * 100

    p0_list.append(p0)
    p1_list.append(p1)

# ---- GRAPH ----
plt.plot(runs_list, p0_list, marker='o', label='0 probability %')
plt.plot(runs_list, p1_list, marker='o', label='1 probability %')

plt.xscale("log")
plt.xlabel("Number of Runs (log scale)")
plt.ylabel("Percentage (%)")
plt.title("Quantum Measurement Convergence (Hadamard Qubit)")
plt.legend()
plt.grid(True)

plt.show()
