# Documentation and CHM Release Checklist

Every new RSDK version must update documentation **in place**. Filenames for the CHM project, compiled CHM, and top-level Markdown docs must remain **versionless** (to avoid leaving 5.2.1/5.2.2/5.2.3 copies behind).

1. Update `Docs/Getting_Started.md`, `Docs/API_Reference.md`, `Docs/CHANGELOG.md` and `Docs/VALIDATION.md` for the new version.
2. Update `Docs/GettingStarted/*.md` and code examples for all changed public APIs.
3. Regenerate the matching `Docs/CHM_Project/html/...` pages; verify APIs, links and examples are current.
4. Update the **contents** of `Docs/CHM_Project/EonCrest_RSDK.hhp`, `.hhc`, `.hhk`, without renaming the files.
5. Set the current version inside `Docs/CHM_Project/CHM_VERSION.txt` and update the HHP documentation title.
6. Run `Docs/CHM_Project/Build_CHM.bat <VERSION>` with HTML Help Workshop on Windows; it must output `Docs/EonCrest_RSDK.chm`.
7. Run `BuildRelease.bat` on Windows VS2022. The script must reject mismatched documentation versions, validate the newly compiled CHM signature and package only `EonCrest_RSDK.chm`.
8. Check SDK ABI / plugin-provider version tables and all linked API signatures against Public headers.
9. Do not ship stale or manually renamed `.chm` files; do not create new numbered `.hhp`, `.hhc`, `.hhk` or Markdown documentation copies.
