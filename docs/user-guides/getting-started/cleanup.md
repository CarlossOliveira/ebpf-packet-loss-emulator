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

<div style="display:flex; justify-content:space-between;">
<a href="usage.md"
   style="
      display:inline-block;
      padding:6px 16px;
      border:1px solid #dadde1;
      border-radius:8px;
      text-decoration:none;
      line-height:1.4;
   ">

   <div style="font-size:0.75rem; color:#6b7280;">
      Previous
   </div>
   <div style="font-weight:600;">
      ← Usage
   </div>
</a>

<a href="../../source/architecture/architecture.md"
   style="
      display:inline-block;
      padding:6px 16px;
      border:1px solid #dadde1;
      border-radius:8px;
      text-decoration:none;
      text-align:right;
      line-height:1.4;
   ">

   <div style="font-size:0.75rem; color:#6b7280;">
      Next
   </div>
   <div style="font-weight:600;">
      Architecture Overview →
   </div>
</a>
</div>
