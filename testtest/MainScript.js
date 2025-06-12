//#region VARIABLES

blockname = "product.data.performanceDesc:35";
largerblock = "product.data.performanceDesc:36";
dividerblocks = "product.data.performanceDesc:37";
revistedblocks = "product.data.performanceDesc:39";
dividerblocks2 = "data.performanceDesc:37";
dipDividerBlock = "data.performanceDesc:45";
assortAdd = "product.data.performanceFlag:6";
pog_store_number = "data.desc:49";
dividerWidth = in2M(0.5);
dividerTolerance = in2M(0.11);
dividerWidthWithT = dividerWidth + dividerTolerance; // add a tolerance

templates_NO_EP_NO_505_YES_YUM = [
  { size: 50, uuid: "f7dcfe93-105a-4250-b9c0-5cd18ab34d21" },
  { size: 40, uuid: "bc4ab0c0-c39b-49a9-8c7d-ff607cc93d87" },
];

templates_NO_EP_NO_505_NO_YUM = [
  { size: 72, uuid: "6a4a97a5-23cb-4b2e-b904-7daa3685fdd3" },
  { size: 40, uuid: "2fac423c-737c-4b9f-9f64-42e0b164e0d0" },
  { size: 50, uuid: "27f83649-56bc-47e0-8cc6-ed711c514d89" },
];

templates_NO_EP_YES_505_YES_YUM = [
  { size: 72, uuid: "815bb433-c6f5-4f7e-ad87-44da7c134822" },
  { size: 56, uuid: "c4726aeb-0481-4799-b8d8-2ba506d7af7d" },
  { size: 40, uuid: "6e05be02-eecb-4139-9ffd-3658aa2a2245" },
  { size: 50, uuid: "798ef856-67f9-4a42-9501-d753572ab2ad" },
];

templates_NO_EP_YES_505_NO_YUM = [
  { size: 72, uuid: "4c75859d-4dc5-4592-bef7-9c0cc40ee5f1" },
  { size: 40, uuid: "742b026b-832e-40c9-b9d1-c6a34b89a13f" },
  { size: 56, uuid: "c04cd852-8bcd-4c46-bc0f-4ddf6e779b73" },
];

templates_YES_EP_YES_505_YES_YUM = [
  { size: 56, uuid: "68fd4097-a722-418e-9474-a04c9138af48" },
];

//increased dividerTolerance again y

let variables = {
  flexspace: 1.71,
};

//#endregion

//#region RUN

const cWhite = 0xffffff;
const cRed = 0xff0000;
const cOrange = 0xff9900;
const cYellow = 0xffff00;
const cGreen = 0x00ff00;
const cBlue = 0x0000ff;

async function runHeadless(args) {
  let file = await VqUtils.getFile(args.targetUuid);
  if (file.fileType === "file") {
    let folder = await VqUtils.getFile(file.folderUuid);
    let parentFolder = await VqUtils.getFile(folder.folderUuid);
    let outputFolder = await getOrCreateFolder(
      parentFolder.uuid,
      `${folder.name} - Done 6-5-2025`
    );

    let controller = { signal: { aborted: false } };

    console.log(`Loading target for ${file.name}...`);
    let targetDoc = await loadDoc(file.uuid);

    console.log("Fixing Segments...");

    await renamePOGFlowFix(targetDoc);
    await sleep(50);

    console.log(`Selecting Template...`);

    await sleep(100);
    let targtotSize = Number(targetDoc.data.planogram.data.desc.get(32));
    let targRegionality = targetDoc.data.planogram.data.desc.get(31);
    // let targTempDirectory = targetDoc.data.planogram.data.createdBy
    //let targNotes = targetDoc.data.planogram.data.notes
    let tempRegionality = await getTemplateSizesList(targRegionality);
    let templateDocUUID = await getTemplateUUID2(targtotSize, tempRegionality);

    if (!templateDocUUID) return;

    console.log(templateDocUUID);

    console.log("Loading template...");
    let templateDoc = await loadDoc(templateDocUUID);
    console.log("Preparing...");
    await prepare(targetDoc, templateDoc);

    await sleep(5000);

    console.log("Optimising...");
    await optimise(targetDoc, controller);

    console.log("Re Optimising Prepare...");
    await reOptPrep(targetDoc);
    console.log("Re Optimising...");
    await reoptimise(targetDoc, controller);

    console.log("Blocking...");
    await blocking(targetDoc);

    // console.log("Re Optimising Prepare...")
    // await reOptPrep(targetDoc)
    // console.log("Re Optimising...")
    // await reoptimise(targetDoc, controller);

    console.log("Sub Planogram Preparing...");
    await subPlanogramPrepare(targetDoc, templateDoc);

    console.log("Tidying...");
    await tidy(targetDoc);

    console.log("Saving...");
    let outputBlob = await RplanUtils.export(targetDoc, "psa");
    if (targetDoc.data.planogram.data.desc.get(50) === "OVER-ALLOCATED") {
      await VqUtils.createFile(
        file.name.replace(".psa", " - OVERALLOCATED") + ".psa",
        outputFolder.uuid,
        outputBlob,
        true
      );
    } else {
      targetDoc.data.planogram.data.desc.set(50, "PROCESSED");

      await VqUtils.createFile(file.name, outputFolder.uuid, outputBlob, true);
    }
    console.log("Finished");
  }
}

async function runLocal() {
  const docs = Array.from(app.sys.windows.values()).map((w) => w.doc);
  const targetDoc = app.sys.view.doc;
  const templateDoc = docs.find(
    (d) => d !== targetDoc && d.data.planogram.positions.size > 0
  );

  let controller = new AbortController();

  await requireBothDocs(targetDoc, templateDoc);
  await prepare(targetDoc, templateDoc);
  await optimise(targetDoc, controller);
  await blocking(targetDoc);
  await tidy(targetDoc);
}

async function run() {
  docs = Array.from(app.sys.windows.values()).map((w) => w.doc);
  targetDoc = app.sys.view.doc;
  templateDoc = docs.find(
    (d) => d !== targetDoc && d.data.planogram.positions.size > 0
  );

  let controller = new AbortController();

  const handleEvent = async (e) => {
    let receivedMessage = e.data;
    switch (receivedMessage.type) {
      case "update variable":
        variables[receivedMessage.key] = receivedMessage.value;
        break;
      case "getData":
        docs = Array.from(app.sys.windows.values()).map((w) => w.doc);
        targetDoc = app.sys.view.doc;
        templateDoc2 = docs.find(
          (d) => d !== targetDoc && d.data.planogram.positions.size > 0
        );
        if (templateDoc2) templateDoc = templateDoc2;

        let data = await getData(targetDoc);

        postMessage({ type: "createResults", data, variables: variables }, "*");
        break;
      case "run":
        switch (receivedMessage.step) {
          case "all":
            await requireBothDocs(targetDoc, templateDoc);
            await prepare(targetDoc, templateDoc);
            controller = new AbortController();
            await optimise(targetDoc, controller);
            if (controller.signal.aborted) break;
            await blocking(targetDoc);
            await tidy(targetDoc);
            break;
          case "load Template":
            if (!templateDoc) {
              templateDoc = await loadDoc(
                "bddca443-7346-48e7-a53a-3e3430bf560e"
              );
              if (templateDoc) alert(`Template Loaded`);
              else
                alert(`Failed to process planogram file: ${targetFile.name}`);
            }
            break;
          case "prepare":
            await requireBothDocs(targetDoc, templateDoc);
            await prepare(targetDoc, templateDoc);
            alert(`Preparation Complete`);
            break;
          case "optimise":
            controller = new AbortController();
            await optimise(targetDoc, controller);
            break;
          case "blocking":
            await blocking(targetDoc);
            break;
          case "reoptimisePrepare":
            await reOptPrep(targetDoc);
            break;
          case "reoptimise":
            controller = new AbortController();
            await reoptimise(targetDoc, controller);
            break;
          case "subPlanogramPrepare":
            await requireBothDocs(targetDoc, templateDoc);
            await subPlanogramPrepare(targetDoc, templateDoc);
            alert(`SubPlangrom Preparation Complete`);
            break;
          case "tidy":
            await tidy(targetDoc);
            break;
        }
      case "optimiseAction":
        switch (receivedMessage.step) {
          case "start":
            controller = new AbortController();
            await optimise(targetDoc, controller);
            break;
          case "stop":
            controller.abort();
            break;
          case "next":
            controller = new AbortController();
            optimise(targetDoc, controller);
            controller.abort();
            break;
          case "clearCache":
            clearOptimiseCache();
            break;
        }
        break;
      case "reoptimiseAction":
        switch (receivedMessage.step) {
          case "start":
            controller = new AbortController();
            await reoptimise(targetDoc, controller);
            break;
          case "stop":
            controller.abort();
            break;
          case "next":
            controller = new AbortController();
            reoptimise(targetDoc, controller);
            controller.abort();
            break;
          case "clearCache":
            clearOptimiseCache();
            break;
        }
        break;
      case "highlight":
        switch (receivedMessage.key) {
          case "desc35":
            highlight(blockname);
            break;
          case "desc36":
            highlight(largerblock);
            break;
          case "desc37":
            highlight(dividerblocks);
            break;
          case "desc39":
            highlight(revistedblocks);
            break;
          case "new":
            highlight(assortAdd);
            break;
          case "dims":
            let prodUuids = checkAltEqMinSqu(targetDoc);
            dimsCalc = (pos) =>
              prodUuids.includes(pos.product.uuid) ? cRed : cGreen;
            highlight("dims", dimsCalc);
            break;
          case "dos":
            dosCalc = (pos) => {
              val = pos.planogramProduct.calculatedFields.actualDaysSupply;
              if (val === 0) return cWhite;
              if (val < 3) return cRed;
              if (val >= 3 && val < 7) return cOrange;
              if (val >= 7) return cGreen;
              return cWhite;
            };
            highlight("dos", dosCalc, [
              "planogramProduct.calculatedFields.actualDaysSupply",
            ]);
            break;
          case "packout":
            packoutCalc = (pos) => {
              val =
                pos.planogramProduct.calculatedFields.capacity /
                (Number.isNaN(pos.product.data.value.get(6))
                  ? 1
                  : pos.product.data.value.get(6) === 0
                  ? 1
                  : pos.product.data.value.get(6));
              if (val < 1) return cRed;
              if (val >= 1 && val < 1.5) return cOrange;
              if (val >= 1.5) return cGreen;
              return cWhite;
            };
            highlight("packout", packoutCalc, [
              "planogramProduct.calculatedFields.capacity",
              "product.data.value.6",
            ]);
            break;
          case "prevfacings":
            facingsdiffCalc = (pos) => {
              facings = pos.planogramProduct.calculatedFields.facings;
              prevfacings = Number.isNaN(
                parseFloat(pos.product.data.performanceDesc.get(50))
              )
                ? 1
                : parseFloat(pos.product.data.performanceDesc.get(50));
              return facings === prevfacings ? cGreen : cRed;
            };
            highlight("prevfacings", facingsdiffCalc, [
              "planogramProduct.calculatedFields.facings",
              "product.data.performanceDesc.50",
            ]);
            break;
          case "reset":
            resetHighlights();
            break;
          case "blockingfails":
            blockSizes = await blocking(targetDoc, true);
            facingsdiffCalc = (pos) => {
              let blocks = blockSizes[specialGetUtil(pos, largerblock)];
              if (!blocks) return cWhite;
              let block = blocks[specialGetUtil(pos, dividerblocks)];
              if (!blocks) return cWhite;
              return block.minSize <= block.maxSize ? cGreen : cRed;
            };
            highlightAsync("blockingfails", facingsdiffCalc);
            break;
          case "blockingwidth":
            blockSizes = await blocking(targetDoc, true);
            facingsdiffCalc = (pos) => {
              let blocks = blockSizes[specialGetUtil(pos, largerblock)];
              if (!blocks) return cWhite;
              let totalWidth = Object.values(blocks).reduce(
                (total, z) => total + z.minSize,
                0
              );
              return totalWidth +
                (Object.values(blocks).length - 1) * dividerWidthWithT <=
                pos.fixture.calculatedFields.combinedLinear
                ? cGreen
                : cRed;
            };
            highlightAsync("blockingwidth", facingsdiffCalc);
            break;
          case "facingsmatch":
            facingsMatchCalc = (pos) => {
              let pog = targetDoc.data.planogram;
              const allEqual = (arr) => arr.every((v) => v === arr[0]);
              let data = [];
              for (let [, prodinfo] of pog.productsInfo) {
                if (prodinfo.positions.length > 1) {
                  let posfacings = prodinfo.positions.map(
                    (pos) => pos.facings.x
                  );
                  let isEqual = allEqual(posfacings);
                  if (!isEqual) data.push(prodinfo.product.uuid);
                }
              }
              let colors = {};

              data.forEach(
                (v, index) => (colors[v] = selectColor(index, data.length))
              );

              val = specialGetUtil(pos, "product.uuid");
              color = val in colors ? colors[val] : cWhite;
              return color;
            };
            highlight("facingsmatch", facingsMatchCalc, [
              "planogramProduct.calculatedFields.facings",
            ]);
            break;
        }
        break;
      case "label":
        switch (receivedMessage.key) {
          case "desc35":
            label(blockname);
            break;
          case "desc36":
            label(largerblock);
            break;
          case "desc37":
            label(dividerblocks);
            break;
          case "desc39":
            label(revistedblocks);
            break;
          case "new":
            label(assortAdd);
            break;
          case "dos":
            dosCalc = (pos) =>
              Math.round(
                pos.planogramProduct.calculatedFields.actualDaysSupply * 100
              ) / 100;
            label("dos", dosCalc, [
              "planogramProduct.calculatedFields.actualDaysSupply",
            ]);
            break;
          case "packout":
            packoutcalc = (pos) =>
              Math.round(
                (pos.planogramProduct.calculatedFields.capacity /
                  (Number.isNaN(pos.product.data.value.get(6))
                    ? 1
                    : pos.product.data.value.get(6) === 0
                    ? 1
                    : pos.product.data.value.get(6))) *
                  100
              ) / 100;
            label("packout", packoutcalc, [
              "planogramProduct.calculatedFields.capacity",
              "product.data.value.6",
            ]);
            break;
          case "prevfacings":
            facingsdiffCalc = (pos) => {
              prevfacings = Number.isNaN(
                parseFloat(pos.product.data.performanceDesc.get(50))
              )
                ? 1
                : parseFloat(pos.product.data.performanceDesc.get(50));
              return prevfacings;
            };
            label("prevfacings", facingsdiffCalc, [
              "product.data.performanceDesc.50",
            ]);
            break;
          case "reset":
            resetLabel();
            break;
        }
        break;
      case "condition":
        switch (receivedMessage.key) {
          case "Score":
            scorecalc = (pos) => Math.round(scoringFn(pos) * 100) / 100;
            label("score", scorecalc);
            break;
          default:
            calc = async (pos) => {
              val = await optimise(targetDoc, null, {
                condition: receivedMessage.key,
                pos: pos,
              });
              return val ? 0x00ff00 : 0xff0000;
            };
            highlightAsync(receivedMessage.key, calc);
            break;
        }
        break;
      case "condition2":
        switch (receivedMessage.key) {
          case "Score":
            scorecalc = (pos) => Math.round(scoringFn(pos) * 100) / 100;
            label("score2", scorecalc);
            break;
          default:
            calc = async (pos) => {
              val = await reoptimise(targetDoc, null, {
                condition: receivedMessage.key,
                pos: pos,
              });
              return val ? 0x00ff00 : 0xff0000;
            };
            highlightAsync(receivedMessage.key, calc);
            break;
        }
    }
  };

  const ihtml = cssStyle + body + script;
  const ctx = open(ihtml, handleEvent);
}

//#endregion

//#region UTILS

function in2M(value) {
  return value * 0.0254;
}

function meters2IN(value) {
  return value * 39.3700787;
}

async function loadDoc(targetFileUuid) {
  const targetFile = await VqUtils.getFile(targetFileUuid);
  const blob = await VqUtils.getBlob(targetFile);
  const doc = await RplanUtils.process(blob, targetFile);
  await RplanUtils.sleep(1000);
  if (!doc) return null;
  return doc;
}

async function getDataFromFile(fileUuid, sheetNum = 0) {
  let fileRef = await VqUtils.getFile(fileUuid);
  let fileBlob = await VqUtils.getBlob(fileRef);
  let f = await fileBlob.arrayBuffer();
  const wb = XLSX.read(f);
  const data = XLSX.utils.sheet_to_json(wb.Sheets[wb.SheetNames[sheetNum]], {
    raw: false,
  });
  return data;
}

async function getOrCreateFolder(parentFolderUuid, name) {
  let folder = (await VqUtils.getFilesInFolder(parentFolderUuid)).find(
    (f) => f.name === name
  );
  if (!folder) {
    folder = await VqUtils.createFolder(parentFolderUuid, name);
  }
  return folder;
}

// function to check if pos shouldn't be touched
function leavePosAlone(pos) {
  if (pos.fixture.segment.fixturesIn.size > 5) {
    sorted_fixs = pos.fixture.segment.fixturesIn
      .filter((f) => f.name !== "Bagged Snacks Divider" && f.depth > 0.1)
      .sort((a, b) => a.position.y - b.position.y);
    if (sorted_fixs.at(4) === pos.fixture) return true;
  }
}

async function getData(doc) {
  let pog = doc.data.planogram;

  let prodUsed = 0;
  let prodUnused = 0;
  for (let [, prod] of pog.productsInfo) {
    if (prod.usedStatus === "Used") prodUsed++;
    else prodUnused++;
  }

  data = {
    filename: doc.filename,
    name: pog.name,
    store: pog.data.desc.get(1),
    productCount: prodUsed + prodUnused,
    productsUsed: prodUsed,
    productsUnused: prodUnused,
  };

  return data;
}

async function getTemplateUUID(store_number, folderUuid) {
  const store_POGs = await VqUtils.getFilesInFolder(folderUuid, {
    filter: [{ column: "name", value: `*L${store_number}*` }],
  });
  let nonMultiSpace = 0;
  for (let store_POG of store_POGs) {
    let doc = await loadDoc(store_POG.uuid);
    nonMultiSpace += doc.data.planogram.segments
      .filter((z) => !z.name.includes("MULTI"))
      .reduce((total, a) => total + a.width, 0);
  }
  tempsize = templateSizes
    .filter((z) => z.size <= nonMultiSpace)
    .reduce((total, a) => (a.size >= total ? a.size : total), 0);
  if (tempsize === 0) {
    return;
  }
  return templateSizes.find((t) => t.size === tempsize).uuid;
}

function round2dp(v, dp = 2) {
  return Math.round(v * 10 ** dp) / 10 ** dp;
}

function checkAltEqMinSqu(targetDoc) {
  let failedUuids = new Set();
  for (let [uuid, prod] of targetDoc.data.products) {
    if (
      round2dp(prod.alternateWidth) !==
      round2dp(prod.width * prod.minimumSqueezeFactorX)
    )
      failedUuids.add(uuid);
    if (
      round2dp(prod.alternateHeight) !==
      round2dp(prod.height * prod.minimumSqueezeFactorY)
    )
      failedUuids.add(uuid);
    if (
      round2dp(prod.alternateDepth) !==
      round2dp(prod.depth * prod.minimumSqueezeFactorZ)
    )
      failedUuids.add(uuid);
  }
  return Array.from(failedUuids);
}

async function requireBothDocs(targetDoc, templateDoc) {
  if (!targetDoc || !templateDoc || targetDoc === templateDoc) {
    alert(
      "You need to have 2 planograms. A template with positions and a selected target with positions."
    );
    throw "error";
  }
}

// async function getTemplateSizesList(targRegionality) {
//   if (!targRegionality.includes("YES AM") && !targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && !targRegionality.includes("YES ML")) {
//     return templates_NO_HYPER
//   } else if (!targRegionality.includes("YES AM") && !targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && targRegionality.includes("YES ML")) {
//     return templates_NO_AM__NO_GUYS__NO_BACKERS__YES_ML
//   } else if (!targRegionality.includes("YES AM") && !targRegionality.includes("YES GUYS") && targRegionality.includes("YES BACKERS") && !targRegionality.includes("YES ML")) {
//     return templates_NO_AM__NO_GUYS__YES_BACKERS__NO_ML
//   } else if (!targRegionality.includes("YES AM") && targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && !targRegionality.includes("YES ML")) {
//     return templates_NO_AM__YES_GUYS__NO_BACKERS__NO_ML
//   } else if (!targRegionality.includes("YES AM") && targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && targRegionality.includes("YES ML")) {
//     return templates_NO_AM__YES_GUYS__NO_BACKERS__YES_ML
//   } else if (targRegionality.includes("YES AM") && !targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && !targRegionality.includes("YES ML")) {
//     return templates_YES_AM__NO_GUYS__NO_BACKERS__NO_ML
//   } else if (targRegionality.includes("YES AM") && !targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && targRegionality.includes("YES ML")) {
//     return templates_YES_AM__NO_GUYS__NO_BACKERS__YES_ML
//   } else if (targRegionality.includes("YES AM") && targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && !targRegionality.includes("YES ML")) {
//     return templates_YES_AM__YES_GUYS__NO_BACKERS__NO_ML
//   } else if (targRegionality.includes("YES AM") && targRegionality.includes("YES GUYS") && !targRegionality.includes("YES BACKERS") && targRegionality.includes("YES ML")) {
//     return templates_YES_AM__YES_GUYS__NO_BACKERS__YES_ML
//   }
// }

async function getTemplateSizesList(targRegionality) {
  if (
    !targRegionality.includes("YES YUM") &&
    !targRegionality.includes("YES 505") &&
    !targRegionality.includes("YES EP")
  ) {
    return templatesNO_EP_NO_505_NO_YUM;
  } else if (
    targRegionality.includes("YES YUM") &&
    targRegionality.includes("YES 505") &&
    !targRegionality.includes("YES EP")
  ) {
    return templatesNO_EP_YES_505_YES_YUM;
  } else if (
    targRegionality.includes("YES YUM") &&
    !targRegionality.includes("YES 505") &&
    !targRegionality.includes("YES EP")
  ) {
    return templatesNO_EP_NO_505_YES_YUM;
  } else if (
    !targRegionality.includes("YES YUM") &&
    targRegionality.includes("YES 505") &&
    !targRegionality.includes("YES EP")
  ) {
    return templatesNO_EP_YES_505_NO_YUM;
  } else if (
    targRegionality.includes("YES YUM") &&
    targRegionality.includes("YES 505") &&
    targRegionality.includes("YES EP")
  ) {
    return templatesYES_EP_YES_505_YES_YUM;
  }
}

async function renamePOGFlowFix(targetDoc) {
  let pog = targetDoc.data.planogram;
  if (
    pog.data.desc.get(40).includes("PORK") &&
    !pog.data.desc.get(40).includes("PORK HISP")
  ) {
    let pogReflow = pog.data.desc.get(40).replace("PORK", "PORK HISP");
    pog.data.desc.set(40, pogReflow);
  }
  if (
    pog.data.desc.get(31).includes("PORK") &&
    !pog.data.desc.get(31).includes("PORK HISP")
  ) {
    let storeReflow = pog.data.desc.get(31).replace("PORK", "PORK HISP");
    pog.data.desc.set(31, storeReflow);
  }
}

async function getTemplateUUID2(targtotSize, tempRegionality) {
  tempsize = tempRegionality
    .filter((z) => z.size <= targtotSize)
    .reduce((total, a) => (a.size >= total ? a.size : total), 0);
  if (tempsize === 0) {
    return;
  }
  return tempRegionality.find((t) => t.size === tempsize).uuid;
}

function facingsMatch(doc) {
  let pog = doc.data.planogram;

  const allEqual = (arr) => arr.every((v) => v === arr[0]);

  let data = [];
  for (let [, prodinfo] of pog.productsInfo) {
    if (prodinfo.positions.length > 1) {
      let posfacings = prodinfo.positions.map((pos) => pos.facings.x);
      let isEqual = allEqual(posfacings);
      if (!isEqual) data.push(prodinfo.product.uuid);
    }
  }

  let colors = {};
  data.forEach((v, index) => (colors[v] = selectColor(index, data.length)));

  app.sys.view.project.planogram.positions.forEach((pos) => {
    val = specialGetUtil(pos.data, "product.uuid");
    color = val in colors ? colors[val] : 0xffffff;
    pos.setHighlightColor(color);
  });
}

activeLabelKey = null;

function resetLabel() {
  let view = app.sys.view;
  view.settings.positionLabelOn = false;
  view.settings.positionLabelMethod = null;
  activeLabelKey = null;
}

function label(key, func, tracks) {
  if (activeLabelKey === key) {
    resetLabel();
    return;
  }
  activeLabelKey = key;

  let view = app.sys.view;

  view.settings.positionLabelOn = true;
  view.settings.positionLabelMethod = {
    tracks: func
      ? tracks
        ? getTrackingState(tracks)
        : null
      : getTrackingState([key]),
    label: (node) => {
      let val = func ? func(node.data) : specialGetUtil(node.data, key);
      if (val === false) return;
      return {
        text: val,
        textColor: -16777216,
        bgColor: 4292730333,
        placement: "middle",
        alignment: "center",
        orientation: "horizontal",
      };
    },
  };
}

activeHighlightKey = null;

function resetHighlights() {
  let view = app.sys.view;
  view.settings.positionHighlightMethod = null;
  view.project.planogram.positions.forEach((pos) => {
    pos.setHighlightColor(null);
  });
  activeHighlightKey = null;
}

async function highlightAsync(key, func) {
  if (activeHighlightKey === key) {
    resetHighlights();
    return;
  }

  activeHighlightKey = key;

  app.sys.view.project.planogram.positions.forEach(async (pos) => {
    pos.setHighlightColor(await func(pos.data));
  });
}

function highlight(key, func, tracks) {
  if (activeHighlightKey === key) {
    resetHighlights();
    return;
  }

  activeHighlightKey = key;

  let view = app.sys.view;
  let pog = view.doc.data.planogram;
  let data = new Set();
  for (let position of pog.positions) {
    data.add(specialGetUtil(position, key));
  }
  let colors = data2Colors(Array.from(data), key);
  view.settings.positionHighlightMethod = {
    tracks: func
      ? tracks
        ? getTrackingState(tracks)
        : null
      : getTrackingState([key]),
    color: (pos) => {
      return func
        ? func(pos.data)
        : colors[specialGetUtil(pos.data, key)] ?? cWhite;
    },
  };
}

function data2Colors(data, key) {
  let colors = {};
  data.forEach((v, index) => (colors[v] = selectColor(index, data.length)));
  return colors;
}

function selectColor(colorNum, colors) {
  if (colors < 1) colors = 1; // defaults to one color - avoid divide by zero
  // return hslToHex((colorNum * (270 / colors) % 360), 75, 50);
  return hslToHex(colorNum * 137.508, 75, 50);
}

function hslToHex(h, s, l) {
  l /= 100;
  const a = (s * Math.min(l, 1 - l)) / 100;
  const f = (n) => {
    const k = (n + h / 30) % 12;
    const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
    return Math.round(255 * color)
      .toString(16)
      .padStart(2, "0"); // convert to Hex and prefix "0" if needed
  };
  return parseInt(`${f(0)}${f(8)}${f(4)}`, 16);
}

getTrackingState = (keys) => {
  tracks = {};
  for (let key of keys) {
    _.setWith(tracks, key.replace(":", "."), null, () => {
      return {};
    });
  }
  return { data: tracks };
};

specialGetUtil = (a, key) => {
  let [keyA, keyB] = key.split(":");
  r = keyB ? _.get(a, keyA).get(keyB) : _.get(a, key);
  return r;
};

//#endregion

//#region PREPARE

async function prepare(targetDoc, templateDoc) {
  // from templateDoc
  // 1. read position location
  // 2. read segment names to match to targets
  // 3. Associate any positions in templateDoc with perfflag6 checked to targetDoc
  // 4. copy over pos.product.data.performanceDesc.get(35) & pos.product.data.performanceDesc.get(36) from templateDoc to targetDoc
  // 5. Preserve positions in targetDoc that also have positions in templateDoc plus add positions from templateDoc that have perfflag6 where their pos.product.data.performanceDesc.get(37) exists already within targetDoc

  templateProj = templateDoc.data;
  templatePOG = templateProj.planogram;
  proj = targetDoc.data;
  pog = proj.planogram;
  posits = pog.positions;
  fixs = pog.fixtures;

  desc37sInTarget = pog.data.desc.get(40);

  desc37Groupings = pog.data.desc.get(40).split(";");

  specialGet = (a, key) => {
    let [keyA, keyB] = key.split(":");
    r = keyB ? _.get(a, keyA).get(keyB) : _.get(a, key);
    return r;
  };

  let mvmtOverrideFileRaw = await getDataFromFile(
    "d4480699-3d14-4e0f-95c2-b82f888ad0ce"
  );
  await sleep(50);

  let mvmtOverrideFileStringed = JSON.stringify(mvmtOverrideFileRaw, null);
  let mvmtOverrideFile = JSON.parse(mvmtOverrideFileStringed);

  let kcmsDataFull = await getDataFromFile(
    "76e41527-1afa-4478-8d90-2d82a3e73e3e"
  );
  await sleep(25);

  //console.log(kcmsDataFull)
  kcmsData = kcmsDataFull.filter(
    (z) => z["Store Number"] === specialGet(pog, pog_store_number)
  );
  await sleep(25);

  // set multipack segment name (hack this in for now)
  // function setMultiPackSegmentName() {
  //   for (let seg of pog.segments) {
  //     if (seg.fixturesIn.size === 5) seg.name = "MULTIPACK FULL"
  //   }
  // }

  // setMultiPackSegmentName()

  //Clear extra performance/data records from Target
  //function removeUnused() {
  //for (let [, prodinfo] of pog.productsInfo) {
  //if (prodinfo.usedStatus === "Unused") {
  //targetDoc.data.removeProduct(prodinfo.product);//abc
  //}
  //}
  //}

  //removeUnused()
  await sleep(0);

  // Copy product data from the template to the target
  function copyProductData() {
    //Copy over desc35, desc36, desc37, flag6
    for (let [, product] of proj.products) {
      const tempProduct = templateProj.products.find(
        (p) => p.upc === product.upc
      );
      if (tempProduct) {
        // performance fields
        product.data.performanceDesc.set(
          35,
          tempProduct.data.performanceDesc.get(35)
        );
        product.data.performanceDesc.set(
          36,
          tempProduct.data.performanceDesc.get(36)
        );
        product.data.performanceDesc.set(
          37,
          tempProduct.data.performanceDesc.get(37)
        );
        product.data.performanceDesc.set(
          38,
          tempProduct.data.performanceDesc.get(38)
        );
        product.data.performanceDesc.set(
          39,
          tempProduct.data.performanceDesc.get(39)
        );
        product.data.performanceDesc.set(
          42,
          tempProduct.data.performanceDesc.get(42)
        );
        product.data.performanceDesc.set(
          45,
          tempProduct.data.performanceDesc.get(45)
        );
        product.data.performanceDesc.set(
          46,
          tempProduct.data.performanceDesc.get(46)
        );
        product.data.performanceDesc.set(
          47,
          tempProduct.data.performanceDesc.get(47)
        );
        product.data.performanceFlag.set(
          6,
          tempProduct.data.performanceFlag.get(6)
        );

        // size fields
        product.height = tempProduct.height;
        product.width = tempProduct.width;
        product.depth = tempProduct.depth;
        product.alternateHeight =
          tempProduct.minimumSqueezeFactorY * tempProduct.height;
        product.alternateWidth = tempProduct.width - in2M(0.01);
        product.alternateDepth = tempProduct.depth;
      } else {
        // console.log(`Could not find product: ${product.upc}`)
      }
    }
  }

  copyProductData();
  await sleep(0);

  // Old Find new Items from Template where their Desc37 is on the Target POG
  //function getNewItemsFromTemp() {
  //desc37sInTarget = posits.reduce((total, z) => {
  //desc37group = specialGet(z, dividerblocks)
  //if (!total?.[desc37group])
  //total.push(desc37group)
  //return total
  //}, [])
  //newItems = templatePOG.positions.filter(z => z.product.data.performanceFlag.get(6) && desc37sInTarget.includes(specialGet(z, dividerblocks))).reduce((total, z) => {
  //if (!total?.[z.product])
  //total.push(z.product)
  //return total
  //}, [])
  //
  //for (let prod of newItems) {
  //const targetProduct = proj.products.find((p) => p.upc === prod.upc);
  //if (targetProduct) targetProduct.upc = targetProduct.upc + ' (old)'
  // adds new items
  //const newProdData = prod.valuesByTracker("@copy");
  //const newProduct = targetDoc.data.importProduct({
  //...newProdData,
  //});
  //}
  //}
  function getNewItemsFromTemp() {
    //desc37sInTarget = pog.data.desc.get(40)
    // Removed new item rule
    newItems = templatePOG.positions
      .filter((z) => desc37sInTarget.includes(specialGet(z, dividerblocks)))
      .reduce((total, z) => {
        if (!total?.[z.product]) total.push(z.product);
        return total;
      }, []);
    for (let prod of newItems) {
      const targetProduct = proj.products.find((p) => p.upc === prod.upc);
      //if (targetProduct) targetProduct.upc = targetProduct.upc + ' (old)'
      // adds new items
      const newProdData = prod.valuesByTracker("@copy");
      const newProduct = targetDoc.data.importProduct({
        ...newProdData,
      });
    }
  }

  getNewItemsFromTemp();
  await sleep(0);

  // make sure all product data has come across
  copyProductData();
  await sleep(0);

  function isReversedCheck(list1, list2) {
    let ascCount = 0;
    let descCount = 0;

    const list = list1.map((c) => list2.indexOf(c)).filter((c) => c > 0);

    for (let i = 0; i < list.length - 1; i++) {
      if (list[i] < list[i + 1]) ascCount++;
      else if (list[i] > list[i + 1]) descCount++;
    }

    if (ascCount < descCount) return true;
    else return false;
  }

  // calculate the direction of flow of the target compared to the template - FALL BACK OPTION
  // REVERSE_FLOW = false;
  // function calculateReverseFlow() {
  //   let lowestShelf = pog.fixtures.sort((a, b) => a.position.y - b.position.y).at(0)
  //   let bottomShelfUPCs = pog.positions.filter(pos => pos.fixture.position.y === lowestShelf.position.y).sort((a, b) => a.transform.worldPos.x - b.transform.worldPos.x).map(pos => pos.product.upc)
  //   let lowestShelfTemp = templatePOG.fixtures.sort((a, b) => a.position.y - b.position.y).at(0)
  //   let bottomShelfUPCsTemp = templatePOG.positions.filter(pos => pos.fixture.position.y === lowestShelfTemp.position.y).sort((a, b) => a.transform.worldPos.x - b.transform.worldPos.x).map(pos => pos.product.upc)
  //   REVERSE_FLOW = isReversedCheck(bottomShelfUPCs, bottomShelfUPCsTemp)
  // }

  // calculateReverseFlow()
  // await sleep(0)

  REVERSE_FLOW = pog.data.trafficFlow === 2;

  // remove the dividers
  function dividerRemoval() {
    dividers = pog.fixtures.filter((f) => f.width <= in2M(2));
    for (let div of dividers) {
      div.parent = null;
    }
  }

  dividerRemoval();
  await sleep(0);

  function makePolesNonObstructive() {
    fixturesOfNote = pog.fixtures;
    for (let fix of fixturesOfNote) {
      if (fix.name.includes("POLE")) {
        fix.depth = 0;
        fix.canObstruct = false;
      }
    }
  }

  makePolesNonObstructive();
  await sleep(25);

  //removePositions
  function removeTargetPos() {
    for (let pos of posits) {
      if (leavePosAlone(pos)) continue;
      pos.parent = null;
    }
  }

  removeTargetPos();
  await sleep(0);

  function setCanCombineBottomShelf() {
    botShelf = pog.fixtures.filter(
      (f) =>
        f.width > 0.5 && f.transform.worldPos.y < 0.31 && f.segment.name == ""
    );
    for (let bfix of botShelf) {
      bfix.canCombine = 1;
    }
  }

  setCanCombineBottomShelf();
  await sleep(25);

  function setCanCombineLeftMostSegName() {
    const leftMostXFixbySegName = fixs.filter((z) =>
      Object.values(
        fixs.reduce((total, z) => {
          const segName = _.get(z, "segment.name");
          const valueX = _.get(z, "uiX");

          if (!(segName in total) || valueX < total[segName]) {
            total[segName] = valueX;
          }

          return total;
        }, {})
      ).includes(z.uiX)
    );

    for (let fix of leftMostXFixbySegName) {
      fix.canCombine = 3;
    }
  }

  setCanCombineLeftMostSegName();
  await sleep(0);

  function setMerchDirectionofShelf() {
    fixturesOfNote = pog.fixtures;
    for (let fix of fixturesOfNote) {
      fix.merch.x.direction.value = 0;
    }
  }

  setMerchDirectionofShelf();
  await sleep(25);

  // place the positions based on the Temp
  function copyPosition(position, doc, fixture) {
    const newPosData = position.valuesByTracker("@copy");

    return doc.createByDef(
      {
        type: "Position",
        isRaw: true,
        ...newPosData,
        merchStyle: 4,
        //product: newProduct,
      },
      fixture
    );
  }

  function productsToArray(xs) {
    let rv = [];
    for (let [, x] of xs) {
      rv.push(x);
    }
    return rv;
  }

  //prodsOnTarg = productsToArray(targetDoc.data.products).map(p => p.id + '_' + p.upc)
  //prodsOnTargUPConly = productsToArray(targetDoc.data.products).map(p => p.upc)

  //prodsOnTarg = productsToArray(targetDoc.data.filter(z => desc37sInTarget.includes(specialGet(z, dividerblocks)))).map(p => p.id + '_' + p.upc)
  //prodsOnTargUPConly = productsToArray(targetDoc.data.filter(z => desc37sInTarget.includes(specialGet(z, dividerblocks)))).map(p => p.upc)

  prodsOnTarg = productsToArray(proj.products)
    .filter((z) => desc37sInTarget.includes(specialGet(z, dividerblocks2)))
    .map((p) => p.id + "_" + p.upc);
  prodsOnTargUPConly = productsToArray(proj.products)
    .filter((z) => desc37sInTarget.includes(specialGet(z, dividerblocks2)))
    .map((p) => p.upc);

  // move products from the template to the target
  async function placeProducts() {
    tempSegmentList = templatePOG.segments
      .sort((a, b) => a.uiX - b.uiX)
      .reduce((total, z) => {
        if (!total?.[z.name]) total[z.name] = z;
        return total;
      }, {});
    targSegmentList = pog.segments
      .sort((a, b) => a.uiX - b.uiX)
      .reduce((total, z) => {
        if (!total?.[z.name]) total[z.name] = z;
        return total;
      }, {});
    for (let [segName, segment] of Object.entries(tempSegmentList)) {
      targetSegment = targSegmentList[segName];
      if (!targetSegment) continue;
      targetSegmentFixs = targetSegment.fixturesIn.sort(
        (a, b) => a.position.y - b.position.y
      );
      fixs = segment.fixturesIn.sort((a, b) => a.position.y - b.position.y);
      for (let [index, fix] of fixs.entries()) {
        fix1 = fix.fixtureLeftMost;
        tempPos = templatePOG.positions
          .filter(
            (z) =>
              z.segment.name === segName && z.fixture.fixtureLeftMost === fix1
          )
          .filter((z) =>
            prodsOnTarg.includes(z.id + "_" + z.upc)
              ? true
              : prodsOnTargUPConly.includes(z.upc)
              ? true
              : false
          )
          .sort((a, b) => a.rank.x - b.rank.x);
        targFix = targetSegmentFixs.at(index);
        for (let [posindex, pos] of tempPos.entries()) {
          // this assumes that the template does not have any dividers
          if (leavePosAlone(pos)) continue;
          let oldPosFacings = pos.facings.x;

          let newpos = copyPosition(pos, targetDoc, targFix);
          newpos.rank.x = REVERSE_FLOW
            ? tempPos.length - (posindex + 1)
            : posindex + 1;
          if (
            newpos.product.data.performanceDesc.get(37) === "MULTI" ||
            newpos.product.data.performanceDesc.get(37) === "MULTIPACK" ||
            newpos.product.data.performanceDesc.get(37) === " MULTIPACK"
          ) {
            newpos.facings.x = oldPosFacings;
          } else {
            newpos.facings.x = 1;
          }
        }
        targFix.layoutByRank();
      }
    }
  }

  await sleep(10000);
  placeProducts();
  await sleep(10000);

  pog.updateNodes();

  // set an assumed movement value
  // assumeMvmt = 2.5
  // function assumedMovement() {
  //   for (let pos of posits.filter(z => z.product.data.unitMovement === 0)) {
  //     pos.product.data.unitMovement = assumeMvmt
  //   }
  // }

  // assumedMovement()
  // await sleep(0)

  async function waitForParent(pog) {
    while (true) {
      let allparent = pog.positions.every((v) => v.parent);
      if (allparent) break;
      await sleep(50);
    }
  }

  async function waitForCalcFields(pog) {
    while (true) {
      let allcalcfields = pog.positions.every(
        (v) => v?.planogramProduct?.positionsCount > 0
      );
      if (allcalcfields) break;
      await sleep(50);
    }
  }

  await waitForParent(pog);
  await waitForCalcFields(pog);

  // get posits (positions that we want to optimise)
  posits = pog.positions.filter((z) => !leavePosAlone(z));

  // reset position facings to 1 and set the other merch values
  // function positionReset() {
  //   for (let pos of posits) {
  //     pos.merch.x.placement.value = 3
  //     pos.merch.x.size.value = 1
  //     pos.facings.x = 1
  //   }
  // }

  // positionReset()
  // await sleep(0)

  // reset position facings to 1 and set the other merch values

  /// NEED TO WORK IN CASE INSENSITIVITY TO EXTRA FOR INITIAL INCLUDE
  function positionReset() {
    for (let pos of posits) {
      if (
        pos.product.data.performanceDesc.get(37) === "MULTI" ||
        pos.product.data.performanceDesc.get(37) === "MULTIPACK" ||
        pos.product.data.performanceDesc.get(37) === " MULTIPACK"
      ) {
        pos.merchStyle = 0;
      } else {
        pos.merchStyle = 4;
      }
      pos.merch.x.placement.value = 3;
      pos.merch.x.size.value = 1;
      if (
        pos.product.data.performanceDesc.get(37) === "MULTI" ||
        pos.product.data.performanceDesc.get(37) === "MULTIPACK" ||
        pos.product.data.performanceDesc.get(37) === " MULTIPACK"
      )
        continue;
      if (pos.product.data.performanceDesc.get(38) == "EXTRA") {
        pos.facings.x = 2;
      } else {
        pos.facings.x = 1;
      }
      // if (pos.product.data.performanceDesc.get(38) === "PACKOUT" && Number(pog.data.desc.get(32)) >= 60) {
      //   pos.facings.x = 2
      // }
    }
  }

  positionReset();
  await sleep(1000);

  pog.updateNodes();

  //#region
  //FIGURE OUT WHAT HAPPENED HERE

  function combineDesc37sFromPogDesc40() {
    for (let group of desc37Groupings) {
      let positsInGroup = posits.filter((z) =>
        group.includes(z.product.data.performanceDesc.get(37))
      );
      for (let pos of positsInGroup) {
        pos.product.data.performanceDesc.set(37, group);
      }
    }
  }

  combineDesc37sFromPogDesc40();
  await sleep(10);

  // function desc37anddesc35Reset() {
  //   for (let pos of posits) {
  //     if (desc37sInTarget.includes("TAKI") && pos.product.data.performanceDesc.get(42) === "TAKIS") {
  //       pos.product.data.performanceDesc.set(35, "TAKIS")
  //     }
  //     if (desc37sInTarget.includes("VALUE") && pos.product.data.performanceDesc.get(42) === "VALUE") {
  //       pos.product.data.performanceDesc.set(35, "VALUE")
  //     }

  //   }
  // }

  // desc37anddesc35Reset()
  // await sleep(10)

  //#endregion

  //#region

  // function checkForCombiningOBs() {
  //   if (desc37sInTarget.includes("OB1") && desc37sInTarget.includes("OB2")) {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "OB1" || pos.product.data.performanceDesc.get(37) === "OB2") {
  //         pos.product.data.performanceDesc.set(37, "OB1")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningOBs()
  // await sleep(10)

  // function checkForCombiningGrippos() {
  //   if (desc37sInTarget.includes("GRIPPOS") && desc37sInTarget.includes("LOCAL")) {
  //     //console.log("Grippos and Local")
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "GRIPPOS" || pos.product.data.performanceDesc.get(37) === "LOCAL") {
  //         pos.product.data.performanceDesc.set(37, "LOCAL")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningGrippos()
  // await sleep(10)

  // function checkForCombiningBalreich() {
  //   if (desc37sInTarget.includes("BALREICH") && desc37sInTarget.includes("LOCAL")) {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "BALREICH" || pos.product.data.performanceDesc.get(37) === "LOCAL") {
  //         pos.product.data.performanceDesc.set(37, "LOCAL")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningBalreich()
  // await sleep(10)

  // function checkForCombiningKitchenCooked() {
  //   if (desc37sInTarget.includes("KITCHENCOOKED") && desc37sInTarget.includes("LOCAL")) {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "KITCHENCOOKED" || pos.product.data.performanceDesc.get(37) === "LOCAL") {
  //         pos.product.data.performanceDesc.set(37, "LOCAL")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningKitchenCooked()
  // await sleep(10)

  // function checkForCombiningCoreCheeto() {
  //   if (desc37sInTarget.includes("DORITOS") && desc37sInTarget.includes("CHEETO") && templatePOG.data.desc.get(50) === "CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "DORITOS" || pos.product.data.performanceDesc.get(37) === "CHEETO") {
  //         pos.product.data.performanceDesc.set(37, "DORITOS")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningCoreCheeto()
  // await sleep(10)

  // function checkForCombiningCoreB4Y() {
  //   if (desc37sInTarget.includes("TOSTITOS") && desc37sInTarget.includes("B4Y") && templatePOG.data.desc.get(50) === "CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "TOSTITOS" || pos.product.data.performanceDesc.get(37) === "B4Y") {
  //         pos.product.data.performanceDesc.set(37, "TOSTITOS")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningCoreB4Y()
  // await sleep(10)

  // function checkForCombiningCoreRINDS() {
  //   if (desc37sInTarget.includes("TAKI") && desc37sInTarget.includes("RINDS") && templatePOG.data.desc.get(50) === "CORE") {
  //     console.log("rinds and Takis")
  //     console.log(posits)
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "TAKI" || pos.product.data.performanceDesc.get(37) === "RINDS") {
  //         pos.product.data.performanceDesc.set(37, "TAKI")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningCoreRINDS()
  // await sleep(10)

  // function checkForCombiningCoreTORTS() {
  //   if (desc37sInTarget.includes("TORTS") && desc37sInTarget.includes("LOCAL") && templatePOG.data.desc.get(50) === "CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "TORTS" || pos.product.data.performanceDesc.get(37) === "LOCAL") {
  //         pos.product.data.performanceDesc.set(37, "LOCAL")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningCoreTORTS()
  // await sleep(10)

  // function checkForCombiningBelowB4Y() {
  //   if (desc37sInTarget.includes("VALUE") && desc37sInTarget.includes("B4Y") && templatePOG.data.desc.get(50) === "BELOW CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "VALUE" || pos.product.data.performanceDesc.get(37) === "B4Y") {
  //         pos.product.data.performanceDesc.set(37, "B4Y")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningBelowB4Y()
  // await sleep(10)

  // function checkForCombiningBelowRINDS() {
  //   if (desc37sInTarget.includes("TAKI") && desc37sInTarget.includes("RINDS") && templatePOG.data.desc.get(50) === "BELOW CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "TAKI" || pos.product.data.performanceDesc.get(37) === "RINDS") {
  //         pos.product.data.performanceDesc.set(37, "TAKI")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningBelowRINDS()
  // await sleep(10)

  // function checkForCombiningBelowCheeto() {
  //   if (desc37sInTarget.includes("DORITOS") && desc37sInTarget.includes("CHEETO") && templatePOG.data.desc.get(50) === "BELOW CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "DORITOS" || pos.product.data.performanceDesc.get(37) === "CHEETO") {
  //         pos.product.data.performanceDesc.set(37, "DORITOS")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningBelowCheeto()
  // await sleep(10)

  // function checkForCombiningBelowTORTS() {
  //   if (desc37sInTarget.includes("TORTS") && desc37sInTarget.includes("LOCAL") && templatePOG.data.desc.get(50) === "BELOW CORE") {
  //     for (let pos of posits) {
  //       if (pos.product.data.performanceDesc.get(37) === "TORTS" || pos.product.data.performanceDesc.get(37) === "LOCAL") {
  //         pos.product.data.performanceDesc.set(37, "LOCAL")
  //       }
  //     }
  //   }
  // }

  // checkForCombiningBelowTORTS()
  // await sleep(10)

  // function desc37anddesc35Reset() {
  //   for (let pos of posits) {
  //     if (desc37sInTarget.includes("TAKI") && pos.product.data.performanceDesc.get(42) === "TAKIS") {
  //       pos.product.data.performanceDesc.set(35, "TAKIS")
  //     }
  //     if (desc37sInTarget.includes("VALUE") && pos.product.data.performanceDesc.get(42) === "VALUE") {
  //       pos.product.data.performanceDesc.set(35, "VALUE")
  //     }

  //   }
  // }

  // desc37anddesc35Reset()
  // await sleep(10)

  //#endregion
  function positionDataUpdate0() {
    for (let pos of pog.positions) {
      let positionDATAset = kcmsData.filter(
        (z) => z["Base BAS_CON_UPC_NO"] === pos.product.upc
      );
      let positionDATA = positionDATAset[0];
      if (positionDATAset.length > 0) {
        //console.log(Number(positionDATA["Avg Weekly Margin"]))
        pos.product.data.performanceValue.set(
          1,
          positionDATA["Avg Weekly Mvmt"] !== null &&
            positionDATA["Avg Weekly Mvmt"] !== undefined
            ? Number(
                positionDATA["Avg Weekly Mvmt"].includes(",")
                  ? positionDATA["Avg Weekly Mvmt"].replaceAll(",", "")
                  : positionDATA["Avg Weekly Mvmt"]
              )
            : 0
        ); //Avg Weekly Mvmt
        pos.product.data.performanceValue.set(
          2,
          positionDATA["Avg Weekly Sales"] !== null &&
            positionDATA["Avg Weekly Sales"] !== undefined
            ? Number(
                positionDATA["Avg Weekly Sales"].includes(",")
                  ? positionDATA["Avg Weekly Sales"].replaceAll(",", "")
                  : positionDATA["Avg Weekly Sales"]
              )
            : 0
        ); //Avg Weekly Sales
        pos.product.data.performanceValue.set(
          3,
          positionDATA["Avg Weekly Margin"] !== null &&
            positionDATA["Avg Weekly Margin"] !== undefined
            ? Number(
                positionDATA["Avg Weekly Margin"].includes(",")
                  ? positionDATA["Avg Weekly Margin"].replaceAll(",", "")
                  : positionDATA["Avg Weekly Margin"]
              )
            : 0
        ); //Avg Weekly Margin
        pos.product.data.performanceValue.set(
          4,
          positionDATA["26wk Avg Weekly Mvmt"] !== null &&
            positionDATA["26wk Avg Weekly Mvmt"] !== undefined
            ? Number(
                positionDATA["26wk Avg Weekly Mvmt"].includes(",")
                  ? positionDATA["26wk Avg Weekly Mvmt"].replaceAll(",", "")
                  : positionDATA["26wk Avg Weekly Mvmt"]
              )
            : 0
        ); //Avg Weekly Mvmt 26wk
        pos.product.data.performanceValue.set(
          5,
          positionDATA["26wk Avg Weekly Sales"] !== null &&
            positionDATA["26wk Avg Weekly Sales"] !== undefined
            ? Number(
                positionDATA["26wk Avg Weekly Sales"].includes(",")
                  ? positionDATA["26wk Avg Weekly Sales"].replaceAll(",", "")
                  : positionDATA["26wk Avg Weekly Sales"]
              )
            : 0
        ); //Avg Weekly Sales 26wk
        pos.product.data.performanceValue.set(
          6,
          positionDATA["26wk Avg Weekly Margin"] !== null &&
            positionDATA["26wk Avg Weekly Margin"] !== undefined
            ? Number(
                positionDATA["26wk Avg Weekly Margin"].includes(",")
                  ? positionDATA["26wk Avg Weekly Margin"].replaceAll(",", "")
                  : positionDATA["26wk Avg Weekly Margin"]
              )
            : 0
        ); //Avg Weekly Margin 26wk
        pos.product.data.performanceValue.set(
          7,
          positionDATA["Annual Mvmt"] !== null &&
            positionDATA["Annual Mvmt"] !== undefined
            ? Number(
                positionDATA["Annual Mvmt"].includes(",")
                  ? positionDATA["Annual Mvmt"].replaceAll(",", "")
                  : positionDATA["Annual Mvmt"]
              )
            : 0
        ); //Avg Annual Mvmt
        pos.product.data.performanceValue.set(
          8,
          positionDATA["Annual Sales"] !== null &&
            positionDATA["Annual Sales"] !== undefined
            ? Number(
                positionDATA["Annual Sales"].includes(",")
                  ? positionDATA["Annual Sales"].replaceAll(",", "")
                  : positionDATA["Annual Sales"]
              )
            : 0
        ); //Avg Annual Sales
        pos.product.data.performanceValue.set(
          9,
          positionDATA["Annual Margin"] !== null &&
            positionDATA["Annual Margin"] !== undefined
            ? Number(
                positionDATA["Annual Margin"].includes(",")
                  ? positionDATA["Annual Margin"].replaceAll(",", "")
                  : positionDATA["Annual Margin"]
              )
            : 0
        ); //Avg Annual Margin
        pos.product.data.performanceValue.set(
          10,
          positionDATA["26wk Mvmt"] !== null &&
            positionDATA["26wk Mvmt"] !== undefined
            ? Number(
                positionDATA["26wk Mvmt"].includes(",")
                  ? positionDATA["26wk Mvmt"].replaceAll(",", "")
                  : positionDATA["26wk Mvmt"]
              )
            : 0
        ); //Avg Mvmt 26wk
        pos.product.data.performanceValue.set(
          11,
          positionDATA["26wk Sales"] !== null &&
            positionDATA["26wk Sales"] !== undefined
            ? Number(
                positionDATA["26wk Sales"].includes(",")
                  ? positionDATA["26wk Sales"].replaceAll(",", "")
                  : positionDATA["26wk Sales"]
              )
            : 0
        ); //Avg Sales 26wk
        pos.product.data.performanceValue.set(
          12,
          positionDATA["26wk Margin"] !== null &&
            positionDATA["26wk Margin"] !== undefined
            ? Number(
                positionDATA["26wk Margin"].includes(",")
                  ? positionDATA["26wk Margin"].replaceAll(",", "")
                  : positionDATA["26wk Margin"]
              )
            : 0
        ); //Avg Margin 26wk
        pos.product.data.performanceValue.set(
          17,
          positionDATA["Weeks from First Sold"] !== null &&
            positionDATA["Weeks from First Sold"] !== undefined
            ? Number(
                positionDATA["Weeks from First Sold"].includes(",")
                  ? positionDATA["Weeks from First Sold"].replaceAll(",", "")
                  : positionDATA["Weeks from First Sold"]
              )
            : 0
        ); //Weeks from First Sold
      }
    }
  }

  positionDataUpdate0();
  await sleep(200);

  function positionDataUpdate() {
    for (let pos of posits) {
      pos.product.data.performanceValue.set(
        3,
        pos.product.data.performanceValue.get(3) > 0
          ? pos.product.data.performanceValue.get(3)
          : round2dp(
              0.6 *
                (posits
                  .filter(
                    (z) =>
                      specialGet(z, revistedblocks) ===
                        specialGet(pos, revistedblocks) &&
                      z.product.data.performanceValue.get(3) > 0
                  )
                  .reduce(
                    (total, a) =>
                      total + a.product.data.performanceValue.get(3),
                    0
                  ) /
                  posits.filter(
                    (z) =>
                      specialGet(z, revistedblocks) ===
                        specialGet(pos, revistedblocks) &&
                      z.product.data.performanceValue.get(3) > 0
                  ).length)
            )
      );
      pos.product.data.performanceValue.set(
        2,
        pos.product.data.performanceValue.get(2) > 0
          ? pos.product.data.performanceValue.get(2)
          : round2dp(
              0.6 *
                (posits
                  .filter(
                    (z) =>
                      specialGet(z, revistedblocks) ===
                        specialGet(pos, revistedblocks) &&
                      z.product.data.performanceValue.get(2) > 0
                  )
                  .reduce(
                    (total, a) =>
                      total + a.product.data.performanceValue.get(2),
                    0
                  ) /
                  posits.filter(
                    (z) =>
                      specialGet(z, revistedblocks) ===
                        specialGet(pos, revistedblocks) &&
                      z.product.data.performanceValue.get(2) > 0
                  ).length)
            )
      );
      pos.product.data.performanceValue.set(
        1,
        pos.product.data.performanceValue.get(1) > 0
          ? pos.product.data.performanceValue.get(1)
          : round2dp(
              0.6 *
                (posits
                  .filter(
                    (z) =>
                      specialGet(z, revistedblocks) ===
                        specialGet(pos, revistedblocks) &&
                      z.product.data.performanceValue.get(1) > 0
                  )
                  .reduce(
                    (total, a) =>
                      total + a.product.data.performanceValue.get(1),
                    0
                  ) /
                  posits.filter(
                    (z) =>
                      specialGet(z, revistedblocks) ===
                        specialGet(pos, revistedblocks) &&
                      z.product.data.performanceValue.get(1) > 0
                  ).length)
            )
      );
    }
  }

  positionDataUpdate();
  await sleep(100);

  function removeNaNs() {
    for (let pos of posits) {
      if (isNaN(pos.product.data.performanceValue.get(1))) {
        pos.product.data.performanceValue.set(1, 0);
      }
      if (isNaN(pos.product.data.performanceValue.get(2))) {
        pos.product.data.performanceValue.set(2, 0);
      }
      if (isNaN(pos.product.data.performanceValue.get(3))) {
        pos.product.data.performanceValue.set(3, 0);
      }
    }
  }

  removeNaNs();
  await sleep(10);

  function positionDataUpdate2() {
    for (let pos of posits) {
      if (pos.product.data.performanceValue.get(3) === undefined) {
        pos.product.data.performanceValue.set(3, 0);
      }
      if (pos.product.data.performanceValue.get(2) === undefined) {
        pos.product.data.performanceValue.set(2, 0);
      }
      if (pos.product.data.performanceValue.get(1) === undefined) {
        pos.product.data.performanceValue.set(1, 0);
      }
      if (pos.product.upc === "0002840073740") {
        pos.merch.y.number.value = 1;
        pos.facings.y = 2;
      }
      pos.product.data.unitMovement = pos.product.data.performanceValue.get(1);
    }
  }

  positionDataUpdate2();
  await sleep(100);

  function checkForPartyDipRemoval() {
    for (let pos of posits) {
      if (
        pos.product.data.performanceDesc.get(38).includes("PARTY") &&
        pog.data.notes.includes("NO PARTY")
      ) {
        pos.parent = null;
      }
    }
  }

  checkForPartyDipRemoval();
  await sleep(50);

  function checkForDesc35ReTags() {
    for (let pos of posits) {
      if (
        pos.product.data.performanceDesc.get(38).includes("TAKIS 1") &&
        pog.data.desc.get(40).includes("TAKI")
      ) {
        pos.product.data.performanceDesc.set(35, "TAKIS 1");
      }

      if (
        pos.product.data.performanceDesc
          .get(38)
          .includes("PORK HISP GROUP 1") &&
        pog.data.desc.get(40).includes("PORK")
      ) {
        pos.product.data.performanceDesc.set(35, "PORK HISP GROUP 1");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("TOSTITOS 1") &&
        pog.data.desc.get(40).includes("TOSTI")
      ) {
        pos.product.data.performanceDesc.set(35, "TOSTITOS 1");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("VALUE GROUP 1") &&
        pog.data.desc.get(40).includes("VALUE")
      ) {
        pos.product.data.performanceDesc.set(35, "VALUE GROUP 1");
      }

      if (
        pos.product.data.performanceDesc
          .get(38)
          .includes("PORK HISP GROUP 2") &&
        pog.data.desc.get(40).includes("PORK")
      ) {
        pos.product.data.performanceDesc.set(35, "PORK HISP GROUP 2");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("AO TORTS GROUP 1") &&
        pog.data.desc.get(40).includes("AO TORT")
      ) {
        pos.product.data.performanceDesc.set(35, "AO TORTS GROUP 1");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("AO TORTS GROUP 5") &&
        pog.data.desc.get(40).includes("AO TORT")
      ) {
        pos.product.data.performanceDesc.set(35, "AO TORTS GROUP 5");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("AO CHIPS GROUP 1") &&
        pog.data.desc.get(40).includes("AO CH")
      ) {
        pos.product.data.performanceDesc.set(35, "AO CHIPS GROUP 1");
      }

      if (
        pos.product.data.performanceDesc
          .get(38)
          .includes("CHEETOS FRITOS GROUP 2") &&
        pog.data.desc.get(40).includes("CHEET")
      ) {
        pos.product.data.performanceDesc.set(35, "CHEETOS FRITOS GROUP 2");
      }

      if (
        pos.product.data.performanceDesc
          .get(38)
          .includes("CHEETOS FRITOS GROUP 3") &&
        pog.data.desc.get(40).includes("CHEET")
      ) {
        pos.product.data.performanceDesc.set(35, "CHEETOS FRITOS GROUP 3");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("OB CHIPS GROUP 2") &&
        pog.data.desc.get(40).includes("OB CH")
      ) {
        pos.product.data.performanceDesc.set(35, "OB CHIPS GROUP 2");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("OB CHIPS GROUP 1") &&
        pog.data.desc.get(40).includes("OB CH")
      ) {
        pos.product.data.performanceDesc.set(35, "OB CHIPS GROUP 1");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("LR G 10") &&
        pog.data.desc.get(40).includes("LAYS")
      ) {
        pos.product.data.performanceDesc.set(35, "LR G 10");
      }

      if (
        pos.product.data.performanceDesc.get(38).includes("DORITOS GROUP 2") &&
        pog.data.desc.get(40).includes("DORI")
      ) {
        pos.product.data.performanceDesc.set(35, "DORITOS GROUP 2");
      }
      if (
        pos.product.data.performanceDesc.get(38).includes("AO CHIPS GROUP 3") &&
        pog.data.desc.get(40).includes("AO CH")
      ) {
        pos.product.data.performanceDesc.set(35, "AO CHIPS GROUP 3");
      }
    }
  }

  checkForDesc35ReTags();
  await sleep(50);

  function clearAdHocData() {
    for (let pos of pog.positions) {
      pos.product.data.performanceValue.set(27, 0);

      pos.product.data.performanceValue.set(28, 0);

      pos.product.data.performanceValue.set(29, 0);

      pos.product.data.performanceValue.set(30, 0);
    }
  }

  clearAdHocData();
  await sleep(50);

  // console.log(mvmtOverrideFile.find(z => Number(z["Target_UPC"]) === 2840075904)["Target_UPC"])

  // target and reference UPCs are reversed in the data file...

  function overrideMovement() {
    for (let row of mvmtOverrideFile) {
      let targetUPC = row.Reference_UPC;
      let referenceUPC = row.Target_UPC;
      let overrideFactor = Number(row.Adjustment_Factor);
      if (
        posits.filter((z) => Number(referenceUPC) === Number(z.product.upc))
          .length === 0
      )
        continue;
      if (
        posits.filter((z) => Number(z.product.upc) === Number(targetUPC))
          .length === 0
      )
        continue;
      let overrideproductMvmt = posits
        .find((z) => Number(referenceUPC) === Number(z.product.upc))
        .product.data.performanceValue.get(1);
      let curMvmt = posits
        .find((z) => Number(z.product.upc) === Number(targetUPC))
        .product.data.performanceValue.get(1);
      let overrideMvmt1 = round2dp(overrideFactor * overrideproductMvmt, 2);
      let overrideMvmt = overrideMvmt1 > 0 ? overrideMvmt1 : curMvmt;
      if (overrideproductMvmt > 0) {
        posits
          .find((z) => Number(z.product.upc) === Number(targetUPC))
          .product.data.performanceValue.set(1, overrideMvmt);
      }
    }
  }

  overrideMovement();
  await sleep(50);

  await sleep(500);
  pog.updateNodes;
  console.log("Preparation Complete");
}

//#endregion

//#region OPTIMISE

// scoringFn = pos => {
//   packout = pos.planogramProduct.calculatedFields.capacity / (Number.isNaN(pos.product.data.value.get(6)) ? 1 : (pos.product.data.value.get(6) === 0 ? 1 : pos.product.data.value.get(6)))
//   dos = pos.planogramProduct.calculatedFields.actualDaysSupply
//   facings = pos.planogramProduct.calculatedFields.facings
//   numberOfpositions = pos.planogramProduct.positionsCount
//   prevfacings = (Number.isNaN(parseFloat(pos.product.data.performanceDesc.get(50))) ? 1 : parseFloat(pos.product.data.performanceDesc.get(50)))
//   return (packout > 1.5 ? 29 : 0) + (dos > 7 ? 10 : 0) + (dos > 5 ? 5 : 0) + (dos > 3 ? 3 : 0) + Math.min(50, (((facings + numberOfpositions) / facings) * dos)) + (((facings / numberOfpositions) > 4 ? 2 : 1) * (facings / numberOfpositions)) + ((facings - prevfacings) * ((facings - prevfacings) > 0 ? 5 : .5)) + (parseFloat(pos.product.upc) / 50000000000000)
// }

scoringFn = (pos) => {
  packout =
    pos.planogramProduct.calculatedFields.capacity /
    (Number.isNaN(pos.product.data.value.get(6))
      ? 1
      : pos.product.data.value.get(6) === 0
      ? 1
      : pos.product.data.value.get(6));
  numberOfpositions = pos.planogramProduct.positionsCount;
  facings = pos.planogramProduct.calculatedFields.facings;
  dos2 =
    pos.product.data.performanceValue.get(1) > 0
      ? (pos.planogramProduct.calculatedFields.capacity /
          pos.product.data.performanceValue.get(1)) *
        7
      : 10 * facings;
  dos3 = pos.product.data.performanceDesc.get(38).includes("PARTY")
    ? 500 + 500 / facings
    : dos2;
  obMod =
    pos.product.data.performanceDesc.get(37).includes("OB") &&
    !pos.product.data.performanceDesc.get(38).includes("IGN") &&
    pos.planogramProduct.calculatedFields.facings < 2
      ? 2.1 + dos3 / 100
      : dos3;
  obDOSMod =
    pos.product.data.performanceDesc.get(37).includes("OB") && dos3 < 2
      ? 0.85 + dos3 / 50
      : dos3;
  obDOSMod2DOS =
    pos.product.data.performanceDesc.get(37).includes("OB") &&
    pos.planogramProduct.calculatedFields.facings < 2 &&
    dos3 < 3
      ? 1.95 + dos3 / 100
      : dos3;
  obDOSMod2 =
    pos.product.data.performanceDesc.get(37).includes("OB") && dos3 < 3
      ? 2 + dos3 / 100
      : dos3;
  obModC =
    pos.product.data.performanceDesc.get(38).includes("OB") &&
    pos.planogramProduct.calculatedFields.facings < 2
      ? 2.1 + dos3 / 100
      : dos3;
  obDOSModC =
    pos.product.data.performanceDesc.get(38).includes("OB") && dos3 < 2
      ? 0.85 + dos3 / 50
      : dos3;
  obDOSMod2C =
    pos.product.data.performanceDesc.get(38).includes("OB") && dos3 < 3
      ? 2 + dos3 / 100
      : dos3;
  obDOSMod2DOSC =
    pos.product.data.performanceDesc.get(38).includes("OB") &&
    pos.planogramProduct.calculatedFields.facings < 2 &&
    dos3 < 3
      ? 1.95 + dos3 / 100
      : dos3;
  dos = Math.min(
    dos3,
    obMod,
    obDOSMod,
    obDOSMod2,
    obModC,
    obDOSModC,
    obDOSMod2C,
    obDOSMod2DOS,
    obDOSMod2DOSC
  );
  prevfacings = Number.isNaN(
    parseFloat(pos.product.data.performanceDesc.get(50))
  )
    ? 1
    : parseFloat(pos.product.data.performanceDesc.get(50));
  return (
    ((facings /*+ numberOfpositions */ / facings) * dos3) / 500 +
    (dos > 7 ? 5 : 0) +
    (dos > 5 ? 4 : 0) +
    (dos > 3 ? 3 : 0) +
    (dos > 2.5 ? 2 : 0) +
    (facings /*+ numberOfpositions */ / facings) *
      dos /*+ ((facings - prevfacings) * ((facings - prevfacings) > 0 ? (dos > 2.5 ? 2 : 1) : .5))*/ +
    parseFloat(pos.product.upc) / 50000000000000 +
    parseFloat(pos.transform.worldPos.x) / 90000
  );
};

mapSG = new Map();
mapA = new Map();
mapB = new Map();
mapC = new Map();
mapD = new Map();
mapE = new Map();
mapF = new Map();
mapG = new Map();
mapS = new Map();

clearCache = (...args) => args.forEach((arg) => arg.clear());
clearOptimiseCache = () =>
  clearCache(mapA, mapB, mapC, mapD, mapE, mapF, mapG, mapS);

async function optimise(targetDoc, controller, resolveForPos = null) {
  specialGet = (a, key) => {
    let mapKey = a.uuid + key;
    let r = mapSG.get(mapKey);
    if (r) return r;
    let [keyA, keyB] = key.split(":");
    r = keyB ? _.get(a, keyA).get(keyB) : _.get(a, key);
    mapSG.set(mapKey, r);
    return r;
  };

  proj = targetDoc.data;
  pog = proj.planogram;
  fixs = pog.fixtures;

  // get posits (positions that we want to optimise)
  posits = pog.positions.filter((z) => !leavePosAlone(z));

  reducedPosits = posits.reduce((total, z) => {
    desc36info = specialGet(z, largerblock);
    desc37info = specialGet(z, dividerblocks);
    if (!total[desc36info]) total[desc36info] = {};
    if (!total[desc36info][desc37info]) total[desc36info][desc37info] = [];
    total[desc36info][desc37info].push(z);
    return total;
  }, {});

  conditionMatchBlock = (a, b, group, group2) => {
    block1condition = specialGet(a, group) === specialGet(b, group);
    block2condition = specialGet(a, group2) === specialGet(b, group2);

    return group2 ? block1condition && block2condition : block1condition;
  };

  cached = (map, posit, fn) => {
    let r = map.get(posit);
    if (r) return r;
    r = fn(posit);
    map.set(posit, r);
    return r;
  };

  conditionGfunctionFn = (posit) =>
    round2dp(posit.fixture.calculatedFields.combinedLinear, 6) -
      round2dp(
        specialGet(posit, dividerblocks) === "MULTI" ||
          specialGet(posit, dividerblocks) === "MULTIPACK" ||
          specialGet(posit, dividerblocks) === " MULTIPACK"
          ? 0
          : spaceAvailableModifier(posit),
        6
      ) -
      round2dp(
        specialGet(posit, dividerblocks) === "MULTI"
          ? 0
          : largerBlockDividerSpace(posit),
        6
      ) >=
    0;
  conditionGfunction = (posit) => cached(mapG, posit, conditionGfunctionFn);

  conditionAfunctionFn = (posit) =>
    round2dp(posit.merchSize.x, 6) <
      round2dp(posit.fixture.calculatedFields.combinedLinear, 6) -
        round2dp(
          posits
            .filter(
              (z) =>
                z.fixture.fixtureLeftMost.uuid ===
                posit.fixture.fixtureLeftMost.uuid
            )
            .reduce((total, z) => total + z.merchSize.x * z.facings.x, 0),
          6
        ) && conditionGfunction(posit);
  conditionAfunction = (posit) => cached(mapA, posit, conditionAfunctionFn);

  // conditionAfunctionFn = posit => conditionGfunction(posit) && posit.merchSize.x <= (posit.fixture.calculatedFields.combinedLinear - posits.reduce((total, a) => {
  //   if (a.fixture.fixtureLeftMost.uuid === posit.fixture.fixtureLeftMost.uuid) {
  //     total += (a.merchSize.x * a.facings.x)
  //   }
  //   return total
  // }, 0))
  // conditionAfunction = posit => cached(mapA, posit, conditionAfunctionFn)

  // conditionAfunctionFn = posit => conditionGfunction(posit) && posit.merchSize.x <= (posit.fixture.calculatedFields.combinedLinear - posits.reduce((total, a) => {
  //   if (a.fixture.fixtureLeftMost.uuid === posit.fixture.fixtureLeftMost.uuid) {
  //     total += (a.merchSize.x * a.facings.x)
  //   }
  //   return total
  // }, 0))
  // conditionAfunction = posit => cached(mapA, posit, conditionAfunctionFn)

  spaceAvailableModifier = (posit) =>
    reducedPosits[specialGet(posit, largerblock)] &&
    Object.values(reducedPosits[specialGet(posit, largerblock)]).reduce(
      (total, x) => {
        maxSpaceDesc37 = Object.values(
          x.reduce((total, z) => {
            fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid;
            if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0;
            total[fixtureLeftuuid] += z.merchSize.x * z.facings.x;

            // add an extra facing for the position we are wanting to expand
            if (z === posit) total[fixtureLeftuuid] += posit.merchSize.x;

            return total;
          }, {})
        ).reduce((total, z) => (total > z ? total : z), 0);

        return (total += maxSpaceDesc37);
      },
      0
    );

  largerBlockDividerSpace = (posit) =>
    reducedPosits[specialGet(posit, largerblock)] &&
    Object.values(reducedPosits[specialGet(posit, largerblock)]).reduce(
      (total, x, index) => {
        blockDividerSpace = index === 0 ? 0 : dividerWidthWithT;
        return (total += blockDividerSpace);
      },
      0
    );

  conditionBfunctionFn = (posit) =>
    posit.merchSize.x +
      posits
        .filter(
          (z) =>
            z.fixture.fixtureLeftMost.uuid ===
              posit.fixture.fixtureLeftMost.uuid &&
            conditionMatchBlock(z, posit, blockname, largerblock)
        )
        .reduce((total, z) => total + z.merchSize.x * z.facings.x, 0) -
      in2M(variables.flexspace) <
    Object.values(
      posits
        .filter((z) => conditionMatchBlock(z, posit, blockname, largerblock))
        .reduce((total, z) => {
          fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid;
          if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0;
          total[fixtureLeftuuid] += z.merchSize.x * z.facings.x;
          return total;
        }, {})
    ).reduce((total, z) => (total > z ? total : z), 0);
  conditionBfunction = (posit) => cached(mapB, posit, conditionBfunctionFn);

  conditionCfunctionFn = (posit) =>
    scoring(posit) ===
    posits
      .filter(
        (z) =>
          z.fixture.fixtureLeftMost.uuid ===
            posit.fixture.fixtureLeftMost.uuid &&
          conditionMatchBlock(z, posit, blockname, largerblock) &&
          conditionAfunction(z) &&
          conditionBfunction(z)
      )
      .reduce((total, z) => {
        pscore = scoring(z);
        return total < pscore ? total : pscore;
      }, Infinity);
  conditionCfunction = (posit) => cached(mapC, posit, conditionCfunctionFn);

  // condition D checks that there is a facing available on each shelf of a product group
  conditionDfunctionFn = (posit) =>
    posits
      .filter(
        (z) =>
          conditionMatchBlock(z, posit, blockname, largerblock) &&
          conditionAfunction(z)
      )
      .reduce((total, z) => {
        if (!total.some((item) => item === z.fixture.fixtureLeftMost.uuid)) {
          total.push(z.fixture.fixtureLeftMost.uuid);
        }
        return total;
      }, []).length ===
    posits
      .filter((z) => conditionMatchBlock(z, posit, blockname, largerblock))
      .reduce((total, z) => {
        if (!total.some((item) => item === z.fixture.fixtureLeftMost.uuid)) {
          total.push(z.fixture.fixtureLeftMost.uuid);
        }
        return total;
      }, []).length;
  conditionDfunction = (posit) => cached(mapD, posit, conditionDfunctionFn);

  conditionEfunctionFn = (posit) =>
    scoring(posit) ===
    posits
      .filter(
        (z) =>
          z.fixture.fixtureLeftMost.uuid ===
            posit.fixture.fixtureLeftMost.uuid &&
          conditionMatchBlock(z, posit, blockname, largerblock) &&
          conditionAfunction(z) &&
          conditionDfunction(z)
      )
      .reduce((total, z) => {
        pscore = scoring(z);
        return total < pscore ? total : pscore;
      }, Infinity);
  conditionEfunction = (posit) => cached(mapE, posit, conditionEfunctionFn);

  // condition F is group is the lowest need by group score
  conditionFfunction = (posit) => {
    positsADFn = () =>
      posits.filter(
        (z) =>
          conditionAfunction(z) &&
          conditionDfunction(z) &
            conditionEfunction(z) &
            conditionMatchBlock(z, posit, largerblock, largerblock)
      );
    positsAD = cached(mapF, specialGet(posit, largerblock), positsADFn);

    blockScoreValue = block_Score(
      positsAD.filter((z) =>
        conditionMatchBlock(z, posit, blockname, largerblock)
      )
    );

    positsADGroupsFn = () =>
      positsAD.reduce((total, z) => {
        bname = specialGet(z, blockname);
        lbname = specialGet(z, largerblock);
        group = bname + lbname;
        if (!total.some((item) => item === group)) {
          total.push(group);
        }
        return total;
      }, []);
    positsADGroups = cached(
      mapF,
      specialGet(posit, largerblock) + "group",
      positsADGroupsFn
    );

    bestBlockScoreValue = positsADGroups.reduce((total, z) => {
      bscore = block_Score(
        positsAD.filter(
          (z2) => specialGet(z2, blockname) + specialGet(z2, largerblock) === z
        )
      );
      return total < bscore ? total : bscore;
    }, Infinity);

    return blockScoreValue === bestBlockScoreValue;
  };

  satisfiesBalancingCondition = (posit) => {
    return (
      conditionAfunction(posit) &&
      conditionBfunction(posit) &&
      conditionCfunction(posit)
    );
  };

  satisfiesExpansionCondition = (posit) => {
    return (
      conditionAfunction(posit) &&
      conditionDfunction(posit) &&
      conditionEfunction(posit) &&
      conditionFfunction(posit)
    );
  };

  // Scoring
  scoring = (pos) => cached(mapS, pos, scoringFn);

  if (resolveForPos) {
    switch (resolveForPos.condition) {
      case "A":
        return conditionAfunction(resolveForPos.pos);
      case "B":
        return conditionBfunction(resolveForPos.pos);
      case "C":
        return conditionCfunction(resolveForPos.pos);
      case "D":
        return conditionDfunction(resolveForPos.pos);
      case "E":
        return conditionEfunction(resolveForPos.pos);
      case "F":
        return conditionFfunction(resolveForPos.pos);
      case "G":
        return conditionGfunction(resolveForPos.pos);
      case "Balancing":
        return satisfiesBalancingCondition(resolveForPos.pos);
      case "Expansion":
        return satisfiesExpansionCondition(resolveForPos.pos);
      default:
        break;
    }
  }

  // everything to 1 dos

  // ob to 2 dos

  // everything to 2 dos

  // ob to 3 dos

  // ob to 2 facings

  // everything to 3 dos

  // dos

  // Group Scoring function
  function block_Score(blockGroup) {
    let totalScore = 0;
    let blockLength = 0;
    let minScore = 10000;
    let otherMinScore = 10000;
    for (let pos of blockGroup) {
      itemScore = scoring(pos);
      numberOfpos = pos.planogramProduct.positionsCount;
      facings = pos.planogramProduct.calculatedFields.facings;
      dos2 =
        pos.product.data.performanceValue.get(1) > 0
          ? (pos.planogramProduct.calculatedFields.capacity /
              pos.product.data.performanceValue.get(1)) *
            7
          : 10 * facings;
      dos = pos.product.data.performanceDesc.get(38).includes("PARTY")
        ? 500 + 500 / facings
        : dos2;
      obMod =
        pos.product.data.performanceDesc.get(37).includes("OB") &&
        !pos.product.data.performanceDesc.get(38).includes("IGN") &&
        pos.planogramProduct.calculatedFields.facings < 2
          ? 2.1 + dos / 100
          : 10000;
      obDOSMod =
        pos.product.data.performanceDesc.get(37).includes("OB") && dos < 2
          ? 0.85 + dos / 100
          : itemScore;
      obDOSMod2 =
        pos.product.data.performanceDesc.get(37).includes("OB") && dos < 3
          ? 2 + dos / 100
          : itemScore;
      obDOSMod2DOS =
        pos.product.data.performanceDesc.get(37).includes("OB") &&
        pos.planogramProduct.calculatedFields.facings < 2 &&
        dos < 3
          ? 1.95 + dos / 100
          : itemScore;
      obModC =
        pos.product.data.performanceDesc.get(38).includes("OB") &&
        pos.planogramProduct.calculatedFields.facings < 2
          ? 2.1 + dos / 100
          : 10000;
      obDOSModC =
        pos.product.data.performanceDesc.get(38).includes("OB") && dos < 2
          ? 0.85 + dos / 50
          : itemScore;
      obDOSMod2C =
        pos.product.data.performanceDesc.get(38).includes("OB") && dos < 3
          ? 2 + dos / 100
          : itemScore;
      obDOSMod2DOSC =
        pos.product.data.performanceDesc.get(38).includes("OB") &&
        pos.planogramProduct.calculatedFields.facings < 2 &&
        dos < 3
          ? 1.95 + dos / 100
          : itemScore;
      obModMin2 = Math.min(obModC, obDOSModC, obDOSMod2C);
      obModMin = Math.min(
        obMod,
        obDOSMod,
        obDOSMod2,
        obDOSMod2DOS,
        obDOSMod2DOSC
      );
      overrideScore = Math.min(obModMin2, obModMin);
      posScore = Math.min(dos, overrideScore); //dos < 3.5 ? dos : obModMin
      totalScore += Math.min(10 + itemScore / 250, itemScore) / numberOfpos;
      blockLength += 1 / numberOfpos;
      minScore = Math.min(minScore, posScore);
      otherMinScore = Math.min(otherMinScore, itemScore);
    }
    return minScore + totalScore / blockLength / 500;
  }

  await sleep(0);

  signal = controller.signal;

  // main loop
  while (true) {
    // balancing while loop
    console.log("Balancing step...");

    while (true) {
      clearOptimiseCache();
      positssSatisfyingBalancingCondition = posits
        .filter(satisfiesBalancingCondition)
        .sort((a, b) => scoring(a) - scoring(b));
      if (positssSatisfyingBalancingCondition.length === 0) break;
      oneperFix = positssSatisfyingBalancingCondition.reduce((total, z) => {
        if (
          !total.some(
            (item) =>
              _.get(item, "fixture.fixtureLeftMost.uuid") ===
              _.get(z, "fixture.fixtureLeftMost.uuid")
          )
        ) {
          total.push(z);
        }
        return total;
      }, []);
      for (let pos of oneperFix) {
        pos.facings.x += 1;
      }

      await sleep(0);
    }
    // expansion for loop
    console.log("Expansion step...");

    clearOptimiseCache();
    positssSatisfyingExpansionCondition = posits
      .filter(satisfiesExpansionCondition)
      .sort((a, b) => scoring(a) - scoring(b));
    if (positssSatisfyingExpansionCondition.length === 0) {
      console.log("Finished optimisation...");
      break;
    }
    oneperFix2 = positssSatisfyingExpansionCondition.reduce((total, z) => {
      if (
        !total.some(
          (item) =>
            _.get(item, "fixture.fixtureLeftMost.uuid") ===
            _.get(z, "fixture.fixtureLeftMost.uuid")
        )
      ) {
        total.push(z);
      }
      return total;
    }, []);
    for (let pos of oneperFix2) {
      pos.facings.x += 1;
      //console.log(specialGet(pos, largerblock))
    }
    await sleep(0);

    if (signal.aborted) {
      console.log("Stopped");
      break;
    }
  }
}

//#endregion

//#region BLOCKING

async function blocking(targetDoc, returnCalcs = false) {
  proj = targetDoc.data;
  pog = proj.planogram;
  fixs = pog.fixtures;
  await sleep(100);
  if (
    pog.positions.reduce((total, z) => {
      desc37info = specialGetUtil(z, dividerblocks);
      if (!total.some((item) => item === desc37info)) {
        total.push(desc37info);
      }
      return total;
    }, []).length === 1
  ) {
    console.log("only 1 block, no dividers placed");
  } else {
    createDivider = (x, y, assembly, depth) => {
      targetDoc.createByDef(
        {
          type: "Fixture",
          isRaw: true,
          ftype: 10,
          name: "Bagged Snacks Divider",
          assembly: String(assembly),
          color: "-8355776",
          position: { x: x, y: y, z: 0 },
          width: dividerWidth,
          height: in2M(9),
          depth: depth,
        },
        pog
      );
    };

    // get posits (positions that we want to optimise)
    posits = pog.positions.filter((z) => !leavePosAlone(z));

    getBlockDict = () =>
      posits.reduce((total, z) => {
        desc36info = specialGetUtil(z, largerblock);
        desc37info = specialGetUtil(z, dividerblocks);
        if (!total[desc36info]) total[desc36info] = {};
        if (!total[desc36info][desc37info]) total[desc36info][desc37info] = [];
        total[desc36info][desc37info].push(z);
        return total;
      }, {});

    let reducedPosits = getBlockDict();
    let blockSizes = getBlockDict();

    Object.entries(blockSizes).forEach(([desc36name, desc36block]) => {
      desc37blocks = Object.entries(desc36block);
      desc37blocks.forEach(([desc37name, positions]) => {
        // calculate the minimum x position
        minX = positions.reduce((total, z) => {
          return total < z.transform.worldPos.x
            ? total
            : z.transform.worldPos.x;
        }, {});

        // calculate the maximum width when all the products are squeeze the most
        minSqu = Object.values(
          positions.reduce((total, z) => {
            fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid;
            if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0;
            total[fixtureLeftuuid] += z.merchSize.x * z.facings.x;
            return total;
          }, {})
        ).reduce((total, z) => (total > z ? total : z), 0);

        // calculate the minimum width when all the products are expanded the most
        maxSqu = Object.values(
          positions.reduce((total, z) => {
            fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid;
            if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0;
            total[fixtureLeftuuid] += z.merchSize.x * z.facings.x;
            return total;
          }, {})
        ).reduce((total, z) => (total < z ? total : z), Infinity);

        blockSizes[desc36name][desc37name] = {
          minSize: minSqu,
          maxSize: maxSqu,
          minX,
        };
      });
    });

    Object.entries(blockSizes).forEach(([desc36name, desc36block]) => {
      desc37blocks = Object.entries(desc36block).sort(
        (a, b) => a[1].minX - b[1].minX
      );

      let xPos = 0;
      desc37blocks.forEach(([desc37name, info], index) => {
        block = blockSizes[desc36name][desc37name];

        positions = reducedPosits[desc36name][desc37name];
        fixtures = positions
          .reduce((total, z) => {
            fixtureLeft = z.fixture.fixtureLeftMost;
            if (!total.includes(fixtureLeft)) total.push(fixtureLeft);
            return total;
          }, [])
          .sort((a, b) => b.position.y - a.position.y);

        worldOffset = fixtures[0].transform.worldPos.x;

        block.fixtures = fixtures;
        block.positions = positions;

        // start of the positions
        block.xStart = xPos;
        // end of the positions
        block.xEnd = xPos + block.minSize + dividerTolerance;
        // start of the divider
        block.xDivider = xPos + block.minSize + dividerTolerance + worldOffset;

        xPos += block.minSize + dividerWidthWithT;
      });
    });

    if (returnCalcs) return blockSizes;

    let mainBlock = { MAIN: blockSizes["MAIN"] };

    // remove all the positions
    for (let desc36block of Object.values(mainBlock)) {
      desc37blocks = Object.entries(desc36block).sort(
        (a, b) => a[1].minX - b[1].minX
      );
      for (let [, info] of desc37blocks) {
        for (let pos of info.positions) {
          pos.oldParentUuid = pos.parent.fixtureLeftMost.uuid;
          pos.parent = null;
        }
      }
    }

    for (let [desc36name, desc36block] of Object.entries(mainBlock)) {
      desc37blocks = Object.entries(desc36block).sort(
        (a, b) => a[1].minX - b[1].minX
      );
      numOfBlocks = desc37blocks.length;

      let blockindex = -1;

      for (let [desc37name, info] of desc37blocks) {
        block = blockSizes[desc36name][desc37name];

        blockindex++;

        let fixtures = block.fixtures;
        let positions = block.positions;

        for (let fixture of fixtures) {
          if (blockindex < numOfBlocks - 1)
            createDivider(
              info.xDivider,
              fixture.position.y + fixture.height + in2M(1),
              blockindex,
              fixture.depth
            );

          let fPos = positions
            .filter((p) => p.oldParentUuid === fixture.uuid)
            .sort((a, b) => a.rank.x - b.rank.x);

          let newPosX = block.xStart;
          for (let pos of fPos) {
            pos.parent = fixture;
            pos.position.x = newPosX;
            newPosX += 0.01;
          }
          fixture.layoutByRank();
          await sleep(5);
        }
      }
    }
  }
}

//#endregion

//#region Re-Opt
async function reOptPrep(targetDoc) {
  proj = targetDoc.data;
  pog = proj.planogram;
  fixs = pog.fixtures;
  posits = pog.positions.filter((z) => !leavePosAlone(z));

  specialGet = (a, key) => {
    let mapKey = a.uuid + key;
    let r = mapSG.get(mapKey);
    if (r) return r;
    let [keyA, keyB] = key.split(":");
    r = keyB ? _.get(a, keyA).get(keyB) : _.get(a, key);
    mapSG.set(mapKey, r);
    return r;
  };

  await untidy(targetDoc);
  await sleep(500);

  pog.updateNodes();
  await sleep(50);

  reducedPosits = posits.reduce((total, z) => {
    desc36info = specialGet(z, largerblock);
    desc37info = specialGet(z, dividerblocks);
    if (!total[desc36info]) total[desc36info] = {};
    if (!total[desc36info][desc37info]) total[desc36info][desc37info] = [];
    total[desc36info][desc37info].push(z);
    return total;
  }, {});

  dividerBlocksRecalcWidths = {};

  for (let desc36 of Object.keys(reducedPosits)) {
    for (let dividerGrouping of Object.keys(reducedPosits[desc36])) {
      if (!(desc36 in dividerBlocksRecalcWidths))
        dividerBlocksRecalcWidths[desc36] = {};
      dividerBlocksRecalcWidths[desc36][dividerGrouping] = Object.values(
        reducedPosits[desc36][dividerGrouping].reduce((total, z) => {
          fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid;
          if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0;
          total[fixtureLeftuuid] += z.merchSize.x * z.facings.x;
          return total;
        }, {})
      ).reduce((total, z) => (total > z ? total : z), 0);
    }
  }

  pog.dividerBlocksRecalcWidths = dividerBlocksRecalcWidths;

  for (let pos of posits) {
    pos.merch.x.placement.value = 3;
    pos.merch.x.size.value = 1;
    if (
      pos.product.data.performanceDesc.get(37) === "MULTI" ||
      pos.product.data.performanceDesc.get(37) === "MULTIPACK" ||
      pos.product.data.performanceDesc.get(37) === " MULTIPACK"
    )
      continue;
    if (pos.product.data.performanceDesc.get(38) == "EXTRA") {
      pos.facings.x = 2;
    } else {
      pos.facings.x = 1;
    }
    // if (pos.product.data.performanceDesc.get(38) === "PACKOUT" && Number(pog.data.desc.get(32)) >= 60) {
    //   pos.facings.x = 2
    // }
  }

  await sleep(5);
}

mapSG = new Map();
mapH = new Map();
mapI = new Map();
mapJ = new Map();
mapK = new Map();
mapL = new Map();
mapM = new Map();
mapN = new Map();
mapFinal = new Map();
mapS = new Map();

clearCache = (...args) => args.forEach((arg) => arg.clear());
clearReOptimiseCache = () =>
  clearCache(mapH, mapI, mapJ, mapK, mapL, mapM, mapN, mapFinal, mapS);

async function reoptimise(targetDoc, controller, resolveForPos = null) {
  specialGet = (a, key) => {
    let mapKey = a.uuid + key;
    let r = mapSG.get(mapKey);
    if (r) return r;
    let [keyA, keyB] = key.split(":");
    r = keyB ? _.get(a, keyA).get(keyB) : _.get(a, key);
    mapSG.set(mapKey, r);
    return r;
  };

  proj = targetDoc.data;
  pog = proj.planogram;
  fixs = pog.fixtures;

  // get posits (positions that we want to optimise)
  posits = pog.positions.filter((z) => !leavePosAlone(z));

  // reducedPosits = posits.reduce((total, z) => {
  //   desc36info = specialGet(z, largerblock)
  //   desc37info = specialGet(z, dividerblocks)
  //   if (!total[desc36info])
  //     total[desc36info] = {}
  //   if (!total[desc36info][desc37info])
  //     total[desc36info][desc37info] = []
  //   total[desc36info][desc37info].push(z)
  //   return total
  // }, {})

  // dividerBlocksRecalcWidths = {}

  // for (let desc36 of Object.keys(reducedPosits)) {

  //   for (let dividerGrouping of Object.keys(reducedPosits[desc36])) {
  //     if (!(desc36 in dividerBlocksRecalcWidths)) dividerBlocksRecalcWidths[desc36] = {}
  //     dividerBlocksRecalcWidths[desc36][dividerGrouping] = Object.values(reducedPosits[desc36][dividerGrouping]
  //       .reduce((total, z) => {
  //         fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid
  //         if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0
  //         total[fixtureLeftuuid] += z.merchSize.x * z.facings.x
  //         return total
  //       }, {}))
  //       .reduce((total, z) => (total > z ? total : z), 0)
  //   }
  // }

  dividerBlocksRecalcWidths = pog.dividerBlocksRecalcWidths;

  conditionMatchBlock = (a, b, group, group2) => {
    block1condition = specialGet(a, group) === specialGet(b, group);
    block2condition = specialGet(a, group2) === specialGet(b, group2);

    return group2 ? block1condition && block2condition : block1condition;
  };

  cached = (map, posit, fn) => {
    let r = map.get(posit);
    if (r) return r;
    r = fn(posit);
    map.set(posit, r);
    return r;
  };

  conditionIfunctionFn = (posit) =>
    round2dp(posit.merchSize.x, 6) <=
      round2dp(
        dividerBlocksRecalcWidths[specialGet(posit, largerblock)][
          specialGet(posit, dividerblocks)
        ],
        6
      ) -
        round2dp(
          posits
            .filter(
              (z) =>
                z.fixture.fixtureLeftMost.uuid ===
                  posit.fixture.fixtureLeftMost.uuid &&
                conditionMatchBlock(z, posit, dividerblocks, dividerblocks)
            ) // need to check this part in more detail
            .reduce((total, z) => total + z.merchSize.x * z.facings.x, 0),
          6
        ) +
        0.4 * dividerTolerance &&
    round2dp(posit.merchSize.x, 6) <
      round2dp(posit.fixture.calculatedFields.combinedLinear, 6) -
        round2dp(
          posits
            .filter(
              (z) =>
                z.fixture.fixtureLeftMost.uuid ===
                posit.fixture.fixtureLeftMost.uuid
            )
            .reduce((total, z) => total + z.merchSize.x * z.facings.x, 0),
          6
        );
  conditionIfunction = (posit) => cached(mapI, posit, conditionIfunctionFn);

  conditionJfunctionFn = (posit) =>
    posit.merchSize.x +
      posits
        .filter(
          (z) =>
            z.fixture.fixtureLeftMost.uuid ===
              posit.fixture.fixtureLeftMost.uuid &&
            conditionMatchBlock(z, posit, revistedblocks, dividerblocks)
        )
        .reduce((total, z) => total + z.merchSize.x * z.facings.x, 0) <=
    Object.values(
      posits
        .filter((z) =>
          conditionMatchBlock(z, posit, revistedblocks, dividerblocks)
        )
        .reduce((total, z) => {
          fixtureLeftuuid = z.fixture.fixtureLeftMost.uuid;
          if (!total?.[fixtureLeftuuid]) total[fixtureLeftuuid] = 0;
          total[fixtureLeftuuid] += z.merchSize.x * z.facings.x;
          return total;
        }, {})
    ).reduce((total, z) => (total > z ? total : z), 0);
  conditionJfunction = (posit) => cached(mapJ, posit, conditionJfunctionFn);

  conditionKfunctionFn = (posit) =>
    scoring(posit) ===
    posits
      .filter(
        (z) =>
          z.fixture.fixtureLeftMost.uuid ===
            posit.fixture.fixtureLeftMost.uuid &&
          conditionMatchBlock(z, posit, revistedblocks, dividerblocks) &&
          conditionIfunction(z) &&
          conditionLfunction(z)
      )
      .reduce((total, z) => {
        pscore = scoring(z);
        return total < pscore ? total : pscore;
      }, Infinity);
  conditionKfunction = (posit) => cached(mapK, posit, conditionKfunctionFn);

  conditionNfunctionFn = (posit) =>
    scoring(posit) ===
    posits
      .filter(
        (z) =>
          z.fixture.fixtureLeftMost.uuid ===
            posit.fixture.fixtureLeftMost.uuid &&
          conditionMatchBlock(z, posit, revistedblocks, dividerblocks) &&
          conditionIfunction(z) &&
          conditionJfunction(z)
      )
      .reduce((total, z) => {
        pscore = scoring(z);
        return total < pscore ? total : pscore;
      }, Infinity);
  conditionNfunction = (posit) => cached(mapN, posit, conditionNfunctionFn);

  conditionFinalfunctionFn = (posit) =>
    scoring(posit) ===
    posits
      .filter(
        (z) =>
          z.fixture.fixtureLeftMost.uuid ===
            posit.fixture.fixtureLeftMost.uuid &&
          conditionMatchBlock(z, posit, dividerblocks, dividerblocks) &&
          conditionIfunction(z) &&
          z.planogramProduct.positions.length === 1
      )
      .reduce((total, z) => {
        pscore = scoring(z);
        return total < pscore ? total : pscore;
      }, Infinity);
  conditionFinalfunction = (posit) =>
    cached(mapFinal, posit, conditionFinalfunctionFn);

  conditionLfunctionFn = (posit) =>
    posits
      .filter(
        (z) =>
          conditionMatchBlock(z, posit, revistedblocks, dividerblocks) &&
          conditionIfunction(z)
      )
      .reduce((total, z) => {
        if (!total.some((item) => item === z.fixture.fixtureLeftMost.uuid)) {
          total.push(z.fixture.fixtureLeftMost.uuid);
        }
        return total;
      }, []).length ===
    posits
      .filter((z) =>
        conditionMatchBlock(z, posit, revistedblocks, dividerblocks)
      )
      .reduce((total, z) => {
        if (!total.some((item) => item === z.fixture.fixtureLeftMost.uuid)) {
          total.push(z.fixture.fixtureLeftMost.uuid);
        }
        return total;
      }, []).length;
  conditionLfunction = (posit) => cached(mapL, posit, conditionLfunctionFn);

  conditionMfunction = (posit) => {
    positsILFn = () =>
      posits.filter(
        (z) =>
          conditionIfunction(z) &&
          conditionLfunction(z) &
            conditionKfunction(z) &
            conditionMatchBlock(z, posit, dividerblocks, dividerblocks)
      );
    positsIL = cached(mapM, specialGet(posit, dividerblocks), positsILFn);

    blockScoreValue = block_Score(
      positsIL.filter((z) =>
        conditionMatchBlock(z, posit, revistedblocks, dividerblocks)
      )
    );

    positsILGroupsFn = () =>
      positsIL.reduce((total, z) => {
        bname = specialGet(z, revistedblocks);
        lbname = specialGet(z, dividerblocks);
        group = bname + lbname;
        if (!total.some((item) => item === group)) {
          total.push(group);
        }
        return total;
      }, []);
    positsILGroups = cached(
      mapM,
      specialGet(posit, dividerblocks) + "group",
      positsILGroupsFn
    );

    bestBlockScoreValue = positsILGroups
      .filter((z) => z.includes(specialGet(posit, dividerblocks)))
      .reduce((total, z) => {
        bscore = block_Score(
          positsIL.filter(
            (z2) =>
              specialGet(z2, revistedblocks) + specialGet(z2, dividerblocks) ===
              z
          )
        );
        return total < bscore ? total : bscore;
      }, Infinity);

    // console.log(specialGet(posit, revistedblocks), blockScoreValue, bestBlockScoreValue)

    return blockScoreValue === bestBlockScoreValue;
  };

  satisfiesRevisitedBalancingCondition = (posit) => {
    return (
      conditionIfunction(posit) &&
      conditionJfunction(posit) &&
      conditionNfunction(posit)
    );
  };

  satisfiesRevisitedExpansionCondition = (posit) => {
    return (
      conditionIfunction(posit) &&
      conditionLfunction(posit) &&
      conditionKfunction(posit) &&
      conditionMfunction(posit)
    );
  };

  // Scoring
  scoring = (pos) => cached(mapS, pos, scoringFn);

  if (resolveForPos) {
    switch (resolveForPos.condition) {
      case "I":
        return conditionIfunction(resolveForPos.pos);
      case "J":
        return conditionJfunction(resolveForPos.pos);
      case "K":
        return conditionKfunction(resolveForPos.pos);
      case "L":
        return conditionLfunction(resolveForPos.pos);
      case "M":
        return conditionMfunction(resolveForPos.pos);
      case "N":
        return conditionNfunction(resolveForPos.pos);
      case "H":
        return conditionHfunction(resolveForPos.pos);
      case "Balancing":
        return satisfiesRevisitedBalancingCondition(resolveForPos.pos);
      case "Expansion":
        return satisfiesRevisitedExpansionCondition(resolveForPos.pos);
      default:
        break;
    }
  }

  // Group Scoring function
  function block_Score(blockGroup) {
    let totalScore = 0;
    let blockLength = 0;
    let minScore = 10000;
    let otherMinScore = 10000;
    for (let pos of blockGroup) {
      itemScore = scoring(pos);
      numberOfpos = pos.planogramProduct.positionsCount;
      facings = pos.planogramProduct.calculatedFields.facings;
      dos2 =
        pos.product.data.performanceValue.get(1) > 0
          ? (pos.planogramProduct.calculatedFields.capacity /
              pos.product.data.performanceValue.get(1)) *
            7
          : 10 * facings;
      dos = pos.product.data.performanceDesc.get(38).includes("PARTY")
        ? 500 + 500 / facings
        : dos2;
      obMod =
        pos.product.data.performanceDesc.get(37).includes("OB") &&
        !pos.product.data.performanceDesc.get(38).includes("IGN") &&
        pos.planogramProduct.calculatedFields.facings < 2
          ? 2.1 + dos / 100
          : 10000;
      obDOSMod =
        pos.product.data.performanceDesc.get(37).includes("OB") && dos < 2
          ? 0.85 + dos / 100
          : itemScore;
      obDOSMod2 =
        pos.product.data.performanceDesc.get(37).includes("OB") && dos < 3
          ? 2 + dos / 100
          : itemScore;
      obDOSMod2DOS =
        pos.product.data.performanceDesc.get(37).includes("OB") &&
        pos.planogramProduct.calculatedFields.facings < 2 &&
        dos < 3
          ? 1.95 + dos / 100
          : itemScore;
      obModC =
        pos.product.data.performanceDesc.get(38).includes("OB") &&
        pos.planogramProduct.calculatedFields.facings < 2
          ? 2.1 + dos / 100
          : 10000;
      obDOSModC =
        pos.product.data.performanceDesc.get(38).includes("OB") && dos < 2
          ? 0.85 + dos / 50
          : itemScore;
      obDOSMod2C =
        pos.product.data.performanceDesc.get(38).includes("OB") && dos < 3
          ? 2 + dos / 100
          : itemScore;
      obDOSMod2DOSC =
        pos.product.data.performanceDesc.get(38).includes("OB") &&
        pos.planogramProduct.calculatedFields.facings < 2 &&
        dos < 3
          ? 1.95 + dos / 100
          : itemScore;
      obModMin2 = Math.min(obModC, obDOSModC, obDOSMod2C);
      obModMin = Math.min(
        obMod,
        obDOSMod,
        obDOSMod2,
        obDOSMod2DOS,
        obDOSMod2DOSC
      );
      overrideScore = Math.min(obModMin2, obModMin);
      posScore = Math.min(dos, overrideScore); //dos < 3.5 ? dos : obModMin
      totalScore += Math.min(10 + itemScore / 250, itemScore) / numberOfpos;
      blockLength += 1 / numberOfpos;
      minScore = Math.min(minScore, posScore);
      otherMinScore = Math.min(otherMinScore, itemScore);
    }
    return minScore + totalScore / blockLength / 500;
  }
  await sleep(0);

  signal = controller.signal;

  // main loop
  while (true) {
    // balancing while loop
    console.log("Balancing step...");

    while (true) {
      clearReOptimiseCache();
      positssSatisfyingRevisitedBalancingCondition = posits
        .filter(satisfiesRevisitedBalancingCondition)
        .sort((a, b) => scoring(a) - scoring(b));
      if (positssSatisfyingRevisitedBalancingCondition.length === 0) break;
      oneperFixBlock = positssSatisfyingRevisitedBalancingCondition.reduce(
        (total, z) => {
          if (
            !total.some(
              (item) =>
                _.get(item, "fixture.fixtureLeftMost.uuid") ===
                  _.get(z, "fixture.fixtureLeftMost.uuid") &&
                specialGet(item, dividerblocks) === specialGet(z, dividerblocks)
            )
          ) {
            total.push(z);
          }
          return total;
        },
        []
      );
      for (let pos of oneperFixBlock) {
        pos.facings.x += 1;
      }

      await sleep(0);
    }
    // expansion for loop
    console.log("Expansion step...");

    clearReOptimiseCache();
    positssSatisfyingRevisitedExpansionCondition = posits
      .filter(satisfiesRevisitedExpansionCondition)
      .sort((a, b) => scoring(a) - scoring(b));
    //console.log(mapM)
    if (positssSatisfyingRevisitedExpansionCondition.length === 0) {
      console.log("Finished optimisation...");
      break;
    }
    for (let pos of positssSatisfyingRevisitedExpansionCondition) {
      pos.facings.x += 1;
    }
    await sleep(0);

    if (signal.aborted) {
      console.log("Stopped");
      break;
    }
  }

  // satisfiesFinalConditions = posit => {
  //   return conditionIfunction(posit) && conditionFinalfunction(posit)
  // }
  // while (true) {
  //   clearReOptimiseCache()
  //   positsSatisfyingFinalConditions = posits.filter(satisfiesFinalConditions).sort((a, b) => scoring(a) - scoring(b));
  //   if (positsSatisfyingFinalConditions.length === 0) {
  //     console.log("Finished Final optimisation...")
  //     break
  //   }
  //   for (let pos of positsSatisfyingFinalConditions) {
  //     pos.facings.x += 1
  //   }
  //   await sleep(0)
  // }
}

//#endregion

//#region Sub-Planogram Optimization
async function subPlanogramPrepare(targetDoc, templateDoc) {
  blockname = "product.data.performanceDesc:35";
  largerblock = "product.data.performanceDesc:36";
  dividerblocks = "product.data.performanceDesc:37";
  dividerAltblocks = "product.data.performanceDesc:46";
  revistedblocks = "product.data.performanceDesc:39";
  dividerblocks2 = "data.performanceDesc:37";
  dipDividerBlock = "product.data.performanceDesc:45";
  dipDividerAltBlock = "product.data.performanceDesc:47";
  assortAdd = "product.data.performanceFlag:6";
  dipPosBlock = "desc:30";
  dividerWidth = in2M(0.5);
  dividerTolerance = in2M(0.15);
  templateProj = templateDoc.data;
  templatePOG = templateProj.planogram;
  proj = targetDoc.data;
  pog = targetDoc.data.planogram;
  fixs = targetDoc.data.planogram.fixtures;
  posits = targetDoc.data.planogram.positions;
  dividerWidth = in2M(0.5);

  function in2M(value) {
    return value * 0.0254;
  }

  REVERSE_FLOW = pog.data.trafficFlow === 2;

  function leavePosAlone(pos) {
    if (pos.fixture.segment.fixturesIn.size > 5) {
      sorted_fixs = pos.fixture.segment.fixturesIn
        .filter((f) => f.name !== "Bagged Snacks Divider" && f.depth > 0.1)
        .sort((a, b) => a.position.y - b.position.y);
      if (sorted_fixs.at(4) === pos.fixture) return true;
    }
  }

  mapSG2 = new Map();

  specialGet = (a, key) => {
    let mapKey = a.uuid + key;
    let r = mapSG2.get(mapKey);
    if (r) return r;
    let [keyA, keyB] = key.split(":");
    r = keyB ? _.get(a, keyA).get(keyB) : _.get(a, key);
    mapSG2.set(mapKey, r);
    return r;
  };

  cached = (map, posit, fn) => {
    let r = map.get(posit);
    if (r) return r;
    r = fn(posit);
    map.set(posit, r);
    return r;
  };

  function round2dp(v, dp = 2) {
    return Math.round(v * 10 ** dp) / 10 ** dp;
  }

  await sleep(0);
  //Getting list of Dip Divider Blocks in POG

  // listofDipDividerBlocks = posits.filter(z => specialGet(z, dividerblocks) != "MULTI" && specialGet(z, dividerblocks) != " MULTIPACK" && specialGet(z, dividerblocks) != "MULTIPACK" && !leavePosAlone(z)).reduce((total, pos) => {
  //   posDipDividerBlock = specialGet(pos, dipDividerBlock)
  //   if (!total.includes(posDipDividerBlock)) {
  //     total.push(posDipDividerBlock)
  //   }
  //   return total
  // }, [])

  desc37sInTarget = pog.data.desc.get(40);

  dipItemsInTemplate = templatePOG.positions
    .filter(
      (z) =>
        desc37sInTarget.includes(specialGet(z, dividerblocks)) ||
        desc37sInTarget.includes(specialGet(z, dividerAltblocks))
    )
    .filter((z2) => leavePosAlone(z2));

  dividers = fixs.filter((z) => z.width < 0.1);
  dividersX = dividers.reduce((total, div) => {
    fixX = div.transform.worldPos.x;
    if (!total.includes(fixX)) {
      total.push(fixX);
    }
    return total;
  }, []);

  dips = posits.filter((z) => leavePosAlone(z));
  dipY = dips.reduce(
    (total, dip) =>
      dip.transform.worldPos.y + 0.01 > total
        ? dip.transform.worldPos.y + 0.01
        : total,
    0
  );
  dipFixtureY = dips.reduce(
    (total, pos) =>
      pos.fixture.transform.worldPos.y > total
        ? pos.fixture.transform.worldPos.y
        : total,
    0
  );
  dipFixtures = fixs.filter((z) => dipFixtureY === z.transform.worldPos.y);
  dipDepth = dips.reduce(
    (total, dip) => (dip.fixture.depth > total ? dip.fixture.depth : total),
    0
  );

  function dipMerchSettings() {
    for (let dip of dips) {
      dip.merch.x.size.value = 1;
      dip.merch.x.placement.value = 3;
    }
  }

  dipMerchSettings();
  await sleep(25);

  function delDipDeletesFn() {
    for (let dip of dips) {
      let dipUPC = dip.product.upc;
      if (dipUPC === "0002840069880") {
        dip.parent = null;
      }
    }
  }

  delDipDeletesFn();
  await sleep(50);

  // #region
  // function removeDips() {
  //   for (let dip of dips) {
  //     dip.parent = null
  //   }
  // }

  // removeDips()
  // await sleep(1000)

  // createDivider = (x, y, assembly, depth) => {
  //   targetDoc.createByDef(
  //     {
  //       type: "Fixture",
  //       isRaw: true,
  //       ftype: 10,
  //       name: "dip divider",
  //       assembly: String(assembly),
  //       color: "-8355776",
  //       position: { x: x, y: y, z: 0 },
  //       width: dividerWidth, height: in2M(4.5), depth: depth
  //     },
  //     pog
  //   );
  // }

  // function placeDipDividers() {
  //   for (let dipDivider of listofDipDividerBlocks) {
  //     minXofDipDividers = posits.filter(z => specialGet(z, dipDividerBlock) === dipDivider && !leavePosAlone(z)).reduce((total, z) => z.transform.worldPos.x < total ? z.transform.worldPos.x : total, Infinity)
  //     if (dividersX.filter(z => z <= (minXofDipDividers + .05) && z >= (minXofDipDividers - .1)).length > 0) {
  //       newDipDividerX = dividersX.filter(z => z <= (minXofDipDividers + .05) && z >= (minXofDipDividers - .1)).at(0)
  //       createDivider(newDipDividerX, dipY, "dip divider", dipDepth)
  //     }
  //   }
  // }

  // await sleep(10)
  // placeDipDividers()
  // await sleep(10)

  // function copyPosition(position, doc, fixture) {
  //   const newPosData = position.valuesByTracker("@copy");

  //   return doc.createByDef(
  //     {
  //       type: "Position",
  //       isRaw: true,
  //       ...newPosData,
  //       merchStyle: 0
  //       //product: newProduct,
  //     },
  //     fixture
  //   );
  // }

  // function productsToArray(xs) {
  //   let rv = []
  //   for (let [, x] of xs) {
  //     rv.push(x)
  //   }
  //   return rv
  // };

  // prodsOnTarg = productsToArray(proj.products).filter(z => desc37sInTarget.includes(specialGet(z, dividerblocks2))).map(p => p.id + '_' + p.upc)
  // prodsOnTargUPConly = productsToArray(proj.products).filter(z => desc37sInTarget.includes(specialGet(z, dividerblocks2))).map(p => p.upc)

  // async function placeProducts() {
  //   rankX = 1
  //   for (let dipDivider of listofDipDividerBlocks) {
  //     dipsInBlock = dipItemsInTemplate.filter(z => dipDivider === specialGet(z, dipDividerBlock) || dipDivider === specialGet(z, dipDividerAltBlock))
  //     dipsInBlock = dipsInBlock.sort((a, b) => (a.transform.worldPos.x - b.transform.worldPos.x) * (REVERSE_FLOW ? -1 : 1))
  //     minXofDipDividers = posits.filter(z => specialGet(z, dipDividerBlock) === dipDivider && !leavePosAlone(z)).reduce((total, z) => z.transform.worldPos.x < total ? z.transform.worldPos.x : total, Infinity)
  //     targFix = dipFixtures.find(fix => fix.transform.worldPos.x <= minXofDipDividers && (fix.transform.worldPos.x + fix.width) >= minXofDipDividers)
  //     startX = (minXofDipDividers + .1) - targFix.transform.worldPos.x
  //     for (let pos of dipsInBlock) {
  //       let newpos = copyPosition(pos, targetDoc, targFix)

  //       newpos.position.x = startX
  //       startX += .1
  //       newpos.rank.x = rankX
  //       rankX += 1
  //       if (templatePOG.data.desc.get(50) === "BELOW CORE") {
  //         newpos.facings.x = 2
  //       } else {
  //         newpos.facings.x = 3
  //       }
  //       newpos.desc.set(30, dipDivider)
  //       sleep(15)

  //     }
  //     targFix.layoutByRank();

  //   }
  // }

  // await sleep(1000)
  // placeProducts()
  // await sleep(1000)

  // pog.updateNodes()

  // function copyToPositionDesc() {
  //   for (let pos of posits.filter(z => !leavePosAlone(z))) {
  //     pos.desc.set(30, pos.product.data.performanceDesc.get(45))

  //   }
  // }

  // copyToPositionDesc()
  // await sleep(10)

  // function setDipStyles() {
  //   for (let pos of posits.filter(z => leavePosAlone(z))) {
  //     pos.merch.x.placement.value = 3
  //     pos.merch.x.size.value = 1

  //   }
  // }

  // await sleep(10)
  // setDipStyles()
  // await sleep(10)

  // scoringFn = pos => {
  //   packout = pos.planogramProduct.calculatedFields.capacity / (Number.isNaN(pos.product.data.value.get(6)) ? 1 : (pos.product.data.value.get(6) === 0 ? 1 : pos.product.data.value.get(6)))
  //   numberOfpositions = pos.planogramProduct.positionsCount
  //   facings = pos.planogramProduct.calculatedFields.facings
  //   dos = pos.product.data.performanceValue.get(1) > 0 ? ((pos.planogramProduct.calculatedFields.capacity / pos.product.data.performanceValue.get(1)) * 7) : (5 * (facings / numberOfpositions))
  //   prevfacings = (Number.isNaN(parseFloat(pos.product.data.performanceDesc.get(50))) ? 1 : parseFloat(pos.product.data.performanceDesc.get(50)))
  //   return (packout >= 1.5 ? 9 : 0) + (dos > 7 ? 3 : 0) + (dos > 5 ? 2 : 0) + (dos > 3 ? 1.5 : 0) + (dos > 2 ? 1 : 0) + Math.min(31, (((facings + numberOfpositions) / facings) * dos)) + (((facings /* / numberOfpositions */) > 15 ? 2 : 1) * (facings /* / numberOfpositions */)) + ((facings - prevfacings) * ((facings - prevfacings) > 0 ? 5 : .5)) + (parseFloat(pos.product.upc) / 50000000000000)
  // }

  // scoring = pos => cached(mapS, pos, scoringFn)

  // mapDipEx1 = new Map()
  // mapDipEx2 = new Map()
  // mapDipEx3 = new Map()
  // mapS = new Map()

  // clearCache = (...args) => args.forEach((arg) => arg.clear())
  // clearDipCache = () => clearCache(mapDipEx1, mapDipEx2, mapDipEx3, mapS)

  // async function dipInitialExpansion() {
  //   for (let dipDivider of listofDipDividerBlocks) {
  //     // console.log(dipDivider)
  //     minXDivider = posits.filter(z => specialGet(z, dipDividerBlock) === dipDivider && !leavePosAlone(z)).reduce((total, z) => z.transform.worldPos.x < total ? z.transform.worldPos.x : total, Infinity)
  //     maxXDivider = posits.filter(z => specialGet(z, dipDividerBlock) === dipDivider && !leavePosAlone(z)).reduce((total, z) => (z.transform.worldPos.x + (z.merchSize.x * z.facings.x)) > total ? (z.transform.worldPos.x + (z.merchSize.x * z.facings.x)) : total, 0)
  //     dipsInDivider = targetDoc.data.planogram.positions.filter(z => leavePosAlone(z)).filter(z => specialGet(z, dipPosBlock) === dipDivider)
  //     // console.log(dipsInDivider)

  //     scoringFn = pos => {
  //       packout = pos.planogramProduct.calculatedFields.capacity / (Number.isNaN(pos.product.data.value.get(6)) ? 1 : (pos.product.data.value.get(6) === 0 ? 1 : pos.product.data.value.get(6)))
  //       numberOfpositions = pos.planogramProduct.positionsCount
  //       facings = pos.planogramProduct.calculatedFields.facings
  //       dos = pos.product.data.performanceValue.get(1) > 0 ? ((pos.planogramProduct.calculatedFields.capacity / pos.product.data.performanceValue.get(1)) * 7) : (15 * (facings / numberOfpositions))
  //       prevfacings = (Number.isNaN(parseFloat(pos.product.data.performanceDesc.get(50))) ? 1 : parseFloat(pos.product.data.performanceDesc.get(50)))
  //       return (packout >= 1.5 ? 9 : 0) + (dos > 7 ? 3 : 0) + (dos > 5 ? 2 : 0) + (dos > 3 ? 1.5 : 0) + (dos > 2 ? 1 : 0) + Math.min(150, (((facings + numberOfpositions) / facings) * dos)) +  /* (((facings /* / numberOfpositions ) > 15 ? 2 : 1) * (facings /* / numberOfpositions )) + */ ((facings - prevfacings) * ((facings - prevfacings) > 0 ? 2 : .5)) + (parseFloat(pos.product.upc) / 50000000000000) + (parseFloat(pos.transform.worldPos.x) / 50000)
  //     }

  //     scoring = pos => cached(mapS, pos, scoringFn)

  //     mapDipEx1 = new Map()
  //     mapDipEx2 = new Map()
  //     mapDipEx3 = new Map()
  //     mapS = new Map()

  //     clearCache = (...args) => args.forEach((arg) => arg.clear())
  //     clearDipCache = () => clearCache(mapDipEx1, mapDipEx2, mapDipEx3, mapS)

  //     pog.updateNodes()

  //     conditionDipEx1functionFn = posit => (round2dp(posit.merchSize.x, 6) < (round2dp(posit.fixture.calculatedFields.combinedLinear, 6) - round2dp(posits.filter(z => z.fixture.fixtureLeftMost.uuid === posit.fixture.fixtureLeftMost.uuid).reduce((total, z) => total + z.merchSize.x * z.facings.x, 0), 6)))
  //     conditionDipEx1function = posit => cached(mapDipEx1, posit, conditionDipEx1functionFn)

  //     conditionDipEx2functionFn = posit => (round2dp(maxXDivider, 6) - round2dp(minXDivider, 6)) >= (round2dp(posit.merchSize.x, 6) + round2dp(dipsInDivider.reduce((total, p) => {
  //       return total += p.merchSize.x * p.facings.x
  //     }, 0)))
  //     conditionDipEx2function = posit => cached(mapDipEx2, posit, conditionDipEx2functionFn)

  //     conditionDipEx3functionFn = posit => scoring(posit) === (dipsInDivider
  //       .filter(z => conditionDipEx1function(z) && conditionDipEx2function(z))
  //       .reduce((total, z) => {
  //         pscore = scoring(z)
  //         return total < pscore ? total : pscore
  //       }, Infinity))
  //     conditionDipEx3function = posit => cached(mapDipEx3, posit, conditionDipEx3functionFn)

  //     satisfiesDipExpansionCondition = posit => {
  //       return conditionDipEx1function(posit) && conditionDipEx2function(posit) && conditionDipEx3function(posit)
  //     }

  //     // balancing while loop
  //     console.log("Dip Balancing step...")

  //     while (true) {
  //       clearDipCache()
  //       dipsSatisfyingCondition = targetDoc.data.planogram.positions.filter(z => leavePosAlone(z)).filter(z => specialGet(z, dipPosBlock) === dipDivider).filter(satisfiesDipExpansionCondition).sort((a, b) => scoring(a) - scoring(b));
  //       if (dipsSatisfyingCondition.length === 0) break
  //       oneperFix = dipsSatisfyingCondition.reduce((total, z) => {
  //         if (!total.some(item => z.fixture.fixtureLeftMost.uuid === item.fixture.fixtureLeftMost.uuid)) {
  //           total.push(z)
  //         }
  //         return total
  //       }, [])
  //       for (let pos of oneperFix) {
  //         pos.facings.x += 1
  //         // console.log(pos.facings.x)
  //         clearDipCache()
  //         sleep(0)

  //       }
  //       await sleep(0)

  //     }
  //     // expansion for loop

  //   }

  // }

  // await sleep(5)
  // await dipInitialExpansion()
  // await sleep(10)

  // function dipDividerRemoval() {
  //   dividers = pog.fixtures.filter(f => f.name === "dip divider")
  //   positss = pog.positions.filter(z => leavePosAlone(z))
  //   for (let div of dividers) {
  //     div.parent = null
  //   }
  //   for (let pos of positss) {
  //     pos.merch.x.placement.value = 3
  //   }

  // }

  // dipDividerRemoval()
  // await sleep(10)

  // pog.updateNodes()

  // function layoutDips() {
  //   for (let fix of dipFixtures) {
  //     fix.layoutByRank()
  //   }
  // }

  // await sleep(10)
  // layoutDips()
  // await sleep(10)

  //#endregion

  async function dipFinalExpansion() {
    finalDipSet = targetDoc.data.planogram.positions.filter((z) =>
      leavePosAlone(z)
    );
    // console.log(finalDipSet)

    scoringFn = (pos) => {
      packout =
        pos.planogramProduct.calculatedFields.capacity /
        (Number.isNaN(pos.product.data.value.get(6))
          ? 1
          : pos.product.data.value.get(6) === 0
          ? 1
          : pos.product.data.value.get(6));
      numberOfpositions = pos.planogramProduct.positionsCount;
      facings = pos.planogramProduct.calculatedFields.facings;
      dos =
        pos.product.data.performanceValue.get(1) > 0
          ? (pos.planogramProduct.calculatedFields.capacity /
              pos.product.data.performanceValue.get(1)) *
            7
          : 15 * (facings / numberOfpositions);
      prevfacings = Number.isNaN(
        parseFloat(pos.product.data.performanceDesc.get(50))
      )
        ? 1
        : parseFloat(pos.product.data.performanceDesc.get(50));
      return (
        (packout >= 1.5 ? 9 : 0) +
        (dos > 7 ? 3 : 0) +
        (dos > 5 ? 2 : 0) +
        (dos > 3 ? 1.5 : 0) +
        (dos > 2 ? 1 : 0) +
        Math.min(150, ((facings + numberOfpositions) / facings) * dos) +
        /* (((facings /* / numberOfpositions ) > 15 ? 2 : 1) * (facings /* / numberOfpositions )) + */ (facings -
          prevfacings) *
          (facings - prevfacings > 0 ? 2 : 0.5) +
        parseFloat(pos.product.upc) / 50000000000000 +
        parseFloat(pos.transform.worldPos.x) / 50000
      );
    };

    scoring = (pos) => cached(mapS2, pos, scoringFn);

    mapDipEx4 = new Map();
    mapDipEx5 = new Map();
    mapS2 = new Map();

    clearCache = (...args) => args.forEach((arg) => arg.clear());
    clearDipCache2 = () => clearCache(mapDipEx4, mapDipEx5, mapS2);

    pog.updateNodes();

    conditionDipEx4functionFn = (posit) =>
      round2dp(posit.merchSize.x, 6) <
      round2dp(posit.fixture.calculatedFields.combinedLinear, 6) -
        round2dp(
          posits
            .filter(
              (z) =>
                z.fixture.fixtureLeftMost.uuid ===
                posit.fixture.fixtureLeftMost.uuid
            )
            .reduce((total, z) => total + z.merchSize.x * z.facings.x, 0),
          6
        );
    conditionDipEx4function = (posit) =>
      cached(mapDipEx4, posit, conditionDipEx4functionFn);

    conditionDipEx5functionFn = (posit) =>
      scoring(posit) ===
      finalDipSet
        .filter((z) => conditionDipEx4function(z))
        .reduce((total, z) => {
          pscore = scoring(z);
          return total < pscore ? total : pscore;
        }, Infinity);
    conditionDipEx5function = (posit) =>
      cached(mapDipEx5, posit, conditionDipEx5functionFn);

    satisfiesDipFinalExpansionCondition = (posit) => {
      return conditionDipEx4function(posit) && conditionDipEx5function(posit);
    };

    // // balancing while loop
    console.log("Dip Expansion step...");

    while (true) {
      clearDipCache2();
      dipsSatisfyingConditions = targetDoc.data.planogram.positions
        .filter((z) => leavePosAlone(z))
        .filter(satisfiesDipFinalExpansionCondition)
        .sort((a, b) => scoring(a) - scoring(b));
      if (dipsSatisfyingConditions.length === 0) {
        break;
      } else {
        dipsSatisfyingConditions.at(0).facings.x += 1;
        clearDipCache2();
        sleep(0);
      }
      await sleep(0);
    }
    // expansion for loop
  }

  await sleep(150);
  await dipFinalExpansion();
  await sleep(10);

  function dipFinalMerchSettings() {
    for (let dip of dips) {
      dip.merch.x.size.value = 2;
    }
  }

  dipFinalMerchSettings();
  await sleep(25);
}

//#endregion

//#region TIDY

async function untidy(targetDoc) {
  proj = targetDoc.data;
  pog = proj.planogram;

  // get posits (positions that we want to optimise)
  posits = pog.positions.filter((z) => !leavePosAlone(z));

  function finalMerchSettings() {
    for (let pos of posits) {
      if (
        pos.product.data.performanceDesc.get(37) === "MULTI" ||
        pos.product.data.performanceDesc.get(37) === "MULTIPACK" ||
        pos.product.data.performanceDesc.get(37) === " MULTIPACK"
      ) {
        pos.merchStyle = 0;
      } else {
        pos.merchStyle = 4;
      }
      pos.merch.x.size.value = 1;
      pos.merch.y.placement.value = 2;
      pos.merch.z.placement.value = 2;
    }
  }

  finalMerchSettings();
  await sleep(0);
}

async function tidy(targetDoc) {
  proj = targetDoc.data;
  pog = proj.planogram;
  fixs = pog.fixtures;

  // function removeBottomDividers() {
  //   for (let fix of fixs.filter(f => f.transform.worldPos.y < .254 && f.width < .1)) {
  //     fix.parent = null
  //   }
  // }

  // removeBottomDividers()
  // await sleep(25)

  // get posits (positions that we want to optimise)
  posits = pog.positions;

  function finalMerchSettings() {
    for (let pos of posits) {
      pos.merchStyle = 0;
      if (
        pos.product.data.performanceDesc.get(37) === "MULTI" ||
        pos.product.data.performanceDesc.get(37) === "MULTIPACK" ||
        pos.product.data.performanceDesc.get(37) === " MULTIPACK"
      ) {
        pos.merch.x.size.value = 2;
      } else {
        pos.merch.x.size.value = 2;
      }
      pos.merch.y.placement.value = 2;
      pos.merch.z.placement.value = 2;
    }
  }

  finalMerchSettings();
  await sleep(0);

  function partySizeDipReset() {
    for (let pos of posits) {
      if (pos.product.data.performanceDesc.get(38).includes("PARTY")) {
        pos.facings.x = 1;
      }
    }
  }

  partySizeDipReset();
  await sleep(50);

  pog.merch.z.placement.value = 2;
  pog.merch.y.placement.value = 2;

  async function overAllocatedCheck() {
    pog.data.desc.set(50, "");
    for (let fix of pog.fixtures) {
      if (
        pog.data.desc.get(50) != "OVER-ALLOCATED" &&
        round2dp(fix.calculatedFields.combinedAvailableLinear, 6) < 0
      ) {
        1;
        pog.data.desc.set(50, "OVER-ALLOCATED");
      }
    }
    await sleep(10);
  }

  overAllocatedCheck();
  await sleep(5);
}

//#endregion

//#region UI

var cssStyle = ``;

var body = `
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Planogram Optimizer</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            height: 100vh;
            margin: 0;
            font-size: 14px;
            padding: 10px
        }
        #validationResult {
            width: 100%;
            max-width: 960px;
            padding: 20px;
            background-color: #f8f8f8;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
            text-align: left;
            margin-bottom: 20px; /* Added margin-bottom for spacing */
        }
        .buttons {
            display: flex;
            justify-content: center;
            gap: 20px;
        }
        .button {
            cursor: pointer;
            font-size: 16px;
            color: white;
            background-color: #007BFF;
            border: none;
            padding: 10px 20px;
            text-align: center;
            border-radius: 5px; /* Rounded corners for buttons */
            box-shadow: 2px 2px 10px rgba(0,0,0,0.2); /* subtle shadow for depth */
        }
        .clickable {
            cursor: pointer;
        }
    </style>
</head>
<body>
  <div id="validationResult"></div>
  <div class="buttons">
      <button id="refreshButton" class="button">Refresh</button>
      <button id="resetHighlightButton" class="button">Reset Highlight</button>
  </div>
</body>
`;

var script = `
<script>
  function applySettings() {
    let DOS = document.getElementById("DOS").value;
    let COS = document.getElementById("COS").value;

    let message = {
      type: "run",
      settings: {
        dos: DOS,
        cos: COS,
      },
    };
    window.parent.postMessage(message, "*");
  }

  function addBlockers() {
    let message = {
      type: "blocking",
    };
    window.parent.postMessage(message, "*");
  }

  function receiveMessage(event) {
    const res = JSON.parse(event.data);
    const data = res.data;
    const variables = res.variables;
    switch (res.type) {
      case 'createResults':
        resultSummary(data, variables);
        break;
    }
  }

  function resultSummary(data, variables) {
    const message = '<strong>Bagged Snacks Optimiser</strong>' + '<br>' + '<br>' +
        '<strong>File:</strong> ' + data.filename + '<br>' +
        '<strong>Name:</strong> ' + data.name + '<br>' +
        '<strong>Store:</strong> ' + data.store + '<br>' + '<br>' +
        '<strong>Products:</strong><ul>' +
        '<li>Total: ' + data.productCount + '</li>' +
        '<li>Used: ' + data.productsUsed + '</li>' +
        '<li>Unused: ' + data.productsUnused + '</li></ul>' +
        '<strong>User Overrides</strong>' + 
        '<ol>' +
            '<li>Flex Space (in) <input id="flexspace" onchange="updateVariable(\'flexspace\')" type="number" min="0" value="' + variables.flexspace + '"></li>' +
        '</ol>' +
        '<strong class="clickable" onclick="runStep(\'all\')">Steps: (run all)</strong>' + 
        '<ol>' +
            '<li class="clickable" onclick="runStep(\'load Template\')">Load Template</li>' +
            '<li class="clickable" onclick="runStep(\'prepare\')">Prepare</li>' +
            '<ul>' +
                '<li>New Placed <span class="clickable" onclick="highlight(\'new\')">(H)</span> <span class="clickable" onclick="label(\'new\')">(L)</span></li>' +
                '<li>Desc 35(Block) <span class="clickable" onclick="highlight(\'desc35\')">(H)</span> <span class="clickable" onclick="label(\'desc35\')">(L)</span></li>' +
                '<li>Desc 36(Competitive Space) <span class="clickable" onclick="highlight(\'desc36\')">(H)</span> <span class="clickable" onclick="label(\'desc36\')">(L)</span></li>' +
                '<li>Desc 37(Divider Blocks) <span class="clickable" onclick="highlight(\'desc37\')">(H)</span> <span class="clickable" onclick="label(\'desc37\')">(L)</span></li>' +
                '<li>Desc 39(Revisited Blocks) <span class="clickable" onclick="highlight(\'desc39\')">(H)</span> <span class="clickable" onclick="label(\'desc39\')">(L)</span></li>' +
                '<li>Alt = Squeeze <span class="clickable" onclick="highlight(\'dims\')">(H)</span> <span class="clickable" onclick="label(\'dims\')">(L)</span></li>' +
            '</ul>' + 
            '<li class="clickable" onclick="runStep(\'optimise\')">Optimize</li>' +
            '<ul>' +
                '<li>Conditions</li>' +
                '<ul>' +
                    '<li class="clickable" onclick="highlightCondition(\'A\')">Space Available(A)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'B\')">Blocking Balanced(B)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'C\')">Best in Blocking(C)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'D\')">All Shelves Check(D)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'E\')">Best Expansion Item(E)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'F\')">Best Block Expansion(F)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'G\')">Overall Space Check(G)</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'Balancing\')">Balancing</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'Expansion\')">Expansion</li>' +
                    '<li class="clickable" onclick="highlightCondition(\'Score\')">Score</li>' +
                '</ul>' + 
                '<li>Controls</li>' +
                '<ul>' +
                    '<li class="clickable" onclick="optimiseAction(\'start\')">Start</li>' +
                    '<li class="clickable" onclick="optimiseAction(\'stop\')">Stop</li>' +
                    '<li class="clickable" onclick="optimiseAction(\'next\')">Next</li>' +
                    '<li class="clickable" onclick="optimiseAction(\'clearCache\')">Clear Cache</li>' +
                '</ul>' + 
            '</ul>' + 
            '<li class="clickable" onclick="runStep(\'blocking\')">Blocking</li>' +
            '<ul>' +
                '<li class="clickable" onclick="highlight(\'blockingfails\')">Failures</li>' +
                '<li class="clickable" onclick="highlight(\'blockingwidth\')">Total Width</li>' +
            '</ul>' + 
            '<li class="clickable" onclick="runStep(\'reoptimise\')">reOptimize</li>' +
            '<ul>' +
                '<li class="clickable" onclick="runStep(\'reoptimisePrepare\')">Prepare</li>' +
                '<li>Conditions</li>' +
                '<ul>' +
                    '<li class="clickable" onclick="highlightCondition2(\'I\')">Space Available(I)</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'J\')">Blocking Balanced(J)</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'K\')">Best in Blocking(K)</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'L\')">All Shelves Check(L)</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'N\')">Best Expansion Item(N)</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'M\')">Best Block Expansion(M)</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'Balancing\')">Balancing</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'Expansion\')">Expansion</li>' +
                    '<li class="clickable" onclick="highlightCondition2(\'Score\')">Score</li>' +
                '</ul>' + 
                '<li>Controls</li>' +
                '<ul>' +
                    '<li class="clickable" onclick="reoptimiseAction(\'start\')">Start</li>' +
                    '<li class="clickable" onclick="reoptimiseAction(\'stop\')">Stop</li>' +
                    '<li class="clickable" onclick="reoptimiseAction(\'next\')">Next</li>' +
                    '<li class="clickable" onclick="reoptimiseAction(\'clearCache\')">Clear Cache</li>' +
                '</ul>' + 
            '</ul>' + 
            '<li class="clickable" onclick="runStep(\'subPlanogramPrepare\')">sub-Planogram Prepare</li>' +
            '<li class="clickable" onclick="runStep(\'tidy\')">Tidy</li>' +
        '</ol>' +
        '<strong>Other validations:</strong>' + 
        '<ol>' +
            '<li>DOS <span class="clickable" onclick="highlight(\'dos\')">(H)</span> <span class="clickable" onclick="label(\'dos\')">(L)</span></li>' +
            '<li>Packout <span class="clickable" onclick="highlight(\'packout\')">(H)</span> <span class="clickable" onclick="label(\'packout\')">(L)</span></li>' +
            '<li>Prev-facings <span class="clickable" onclick="highlight(\'prevfacings\')">(H)</span> <span class="clickable" onclick="label(\'prevfacings\')">(L)</span></li>' +
            '<li>Facings mismatch <span class="clickable" onclick="highlight(\'facingsmatch\')">(H)</span></li>'
        '</ol>';

    document.getElementById('validationResult').innerHTML = message;
  }

  function runStep(step) {
    window.parent.postMessage({ type: "run", step: step }, "*");
  }

  function optimiseAction(step) {
    window.parent.postMessage({ type: "optimiseAction", step: step }, "*");
  }

    function reoptimiseAction(step) {
    window.parent.postMessage({ type: "reoptimiseAction", step: step }, "*");
  }

  function highlight(key) {
      window.parent.postMessage({ type: "highlight", key }, "*");
  }

  function label(key) {
      window.parent.postMessage({ type: "label", key }, "*");
  }

  function highlightCondition(key) {
    window.parent.postMessage({ type: "condition", key }, "*");
  }

 function highlightCondition2(key) {
    window.parent.postMessage({ type: "condition2", key }, "*");
  }

  function highlightDetail(key) {
      window.parent.postMessage({ type: "highlight", key }, "*");
  }

  function updateVariable(key) {
      value = document.getElementById(key).value
      window.parent.postMessage({ type: "update variable", key, value }, "*");
  }

  document.getElementById('refreshButton').addEventListener('click', function() {
      window.parent.postMessage({ type: "getData" }, "*");
  });

  document.getElementById('resetHighlightButton').addEventListener('click', function() {
      window.parent.postMessage({ type: "highlight", key: "reset"  }, "*");
      window.parent.postMessage({ type: "label", key: "reset"  }, "*");
  });

  window.parent.postMessage({ type: "getData" }, "*");
</script>
`;

//endregion

// runLocal().then(() => console.log("Done"))
