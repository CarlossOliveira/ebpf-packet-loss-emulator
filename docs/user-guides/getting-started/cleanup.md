# Cleanup

## Remove All Build Artifacts

Delete all generated binaries, object files, and intermediate build artifacts:

```bash
make clean
```

---

## Remove Dependencies

To remove all packages installed by the dependency manager:

Using the installation script:

```bash
sudo ./scripts/install_dependencies.sh uninstall
```

Alternatively:

```bash
sudo make uninstall
```

This operation restores the source tree to a clean state without modifying the project source files.

<p>
  <a href="usage.md">
    <img src="../../assets/images/buttons/prev_usage.svg" align="left">
  </a>
  <a href="../../source/architecture/architecture_overview.md">
    <img src="../../assets/images/buttons/next_architecture_overview.svg" align="right">
  </a>
</p>
