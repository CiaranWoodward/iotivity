/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package oic.simulator.serviceprovider.view.dialogs;

import java.lang.reflect.InvocationTargetException;
import java.util.Date;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.utils.Constants;
import oic.simulator.serviceprovider.utils.Utility;
import oic.simulator.serviceprovider.view.dialogs.MainPage.ResourceOption;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.wizard.IWizardPage;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.oic.simulator.ILogger.Level;
import org.oic.simulator.SimulatorException;

public class UpdatePropertiesPage extends WizardPage {

    private Text   resNameTxt;
    private Text   resUriTxt;

    private String resName;
    private String resURI;

    protected UpdatePropertiesPage() {
        super("Update Properties");
    }

    @Override
    public void createControl(Composite parent) {
        setPageComplete(true);
        setTitle(Constants.UPDATE_PROP_PAGE_TITLE);
        setMessage(Constants.UPDATE_PROP_PAGE_MESSAGE);

        Composite comp = new Composite(parent, SWT.NONE);
        GridLayout gridLayout = new GridLayout();
        comp.setLayout(gridLayout);
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        comp.setLayoutData(gd);

        Group grp = new Group(comp, SWT.NONE);
        gridLayout = new GridLayout(2, false);
        grp.setLayout(gridLayout);
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        grp.setLayoutData(gd);

        Label resNameLbl = new Label(grp, SWT.NULL);
        resNameLbl.setText("Resource Name");
        gd = new GridData();
        gd.verticalIndent = 20;
        resNameLbl.setLayoutData(gd);

        resNameTxt = new Text(grp, SWT.BORDER);
        resNameTxt.setFocus();
        gd = new GridData();
        gd.widthHint = 300;
        gd.verticalIndent = 20;
        resNameTxt.setLayoutData(gd);

        Label resUriLbl = new Label(grp, SWT.NULL);
        resUriLbl.setText("Resource URI");
        gd = new GridData();
        gd.verticalIndent = 10;
        resUriLbl.setLayoutData(gd);

        resUriTxt = new Text(grp, SWT.BORDER);
        gd = new GridData();
        gd.widthHint = 300;
        gd.verticalIndent = 10;
        resUriTxt.setLayoutData(gd);

        Label descLbl = new Label(comp, SWT.NONE);
        descLbl.setText("Description:");
        gd = new GridData();
        descLbl.setLayoutData(gd);

        final Text text = new Text(comp, SWT.MULTI | SWT.READ_ONLY | SWT.BORDER
                | SWT.WRAP | SWT.V_SCROLL);
        text.setText("These properties can be changed later from properties view.");
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        text.setLayoutData(gd);

        addUIListeners();

        // Initialize data
        if (resUriTxt.getText().length() < 1 && null != resURI) {
            resUriTxt.setText(resURI);
        }
        if (resNameTxt.getText().length() < 1 && null != resName) {
            resNameTxt.setText(resName);
        }

        setControl(comp);
    }

    private void addUIListeners() {
        resNameTxt.addModifyListener(new ModifyListener() {
            @Override
            public void modifyText(ModifyEvent e) {
                resName = resNameTxt.getText();
                setPageComplete(isSelectionDone());
                // getWizard().getContainer().updateButtons();
            }
        });

        resUriTxt.addModifyListener(new ModifyListener() {
            @Override
            public void modifyText(ModifyEvent e) {
                resURI = resUriTxt.getText();
                setPageComplete(isSelectionDone());
                // getWizard().getContainer().updateButtons();
            }
        });
    }

    @Override
    public boolean canFlipToNextPage() {
        CreateResourceWizard createWizard = (CreateResourceWizard) getWizard();
        if (isSelectionDone()
                && (createWizard.getMainPage().getResourceOption() == ResourceOption.COLLECTION_FROM_RAML)
                && Activator.getDefault().getResourceManager()
                        .isAnyResourceExist()) {
            return true;
        }
        return false;
    }

    public boolean isSelectionDone() {
        boolean done = false;
        if (null != resName && resName.trim().length() > 0 && null != resURI
                && resURI.trim().length() > 0) {
            done = true;
        }
        return done;
    }

    @Override
    public IWizardPage getNextPage() {
        final boolean done[] = new boolean[1];
        CreateResourceWizard createWizard = (CreateResourceWizard) getWizard();
        // Checking whether the uri is used by any other resource.
        if (Activator.getDefault().getResourceManager().isResourceExist(resURI)) {
            MessageDialog
                    .openError(getShell(), "Resource URI in use",
                            "Entered resource URI is in use. Please try a different one.");
            // TODO: Instead of MessageDialog, errors may be shown on wizard
            // itself.
            return null;
        }
        try {
            getContainer().run(true, true, new IRunnableWithProgress() {

                @Override
                public void run(IProgressMonitor monitor)
                        throws InvocationTargetException, InterruptedException {
                    try {
                        monitor.beginTask(
                                "Completing Collection Resource Creation With RAML",
                                2);
                        monitor.worked(1);
                        done[0] = completeCollectionResourceCreationWithRAML();
                        monitor.worked(1);
                    } finally {
                        monitor.done();
                    }
                }
            });
        } catch (InvocationTargetException e) {
            Activator.getDefault().getLogManager()
                    .log(Level.ERROR.ordinal(), new Date(), e.getMessage());
            e.printStackTrace();
            return null;
        } catch (InterruptedException e) {
            Activator.getDefault().getLogManager()
                    .log(Level.ERROR.ordinal(), new Date(), e.getMessage());
            e.printStackTrace();
            return null;
        }
        if (!done[0]) {
            return null;
        }
        return createWizard.getAddResourcesToCollectionPage();
    }

    public void setResName(String resName) {
        this.resName = resName;
        if (!resNameTxt.isDisposed())
            resNameTxt.setText(resName);
    }

    public void setResURI(String resURI) {
        this.resURI = resURI;
        if (!resUriTxt.isDisposed())
            resUriTxt.setText(resURI);
    }

    public String getResName() {
        return resName;
    }

    public String getResURI() {
        return resURI;
    }

    private boolean completeCollectionResourceCreationWithRAML() {
        boolean result = false;
        String status;
        CreateResourceWizard createWizard = (CreateResourceWizard) getWizard();
        try {
            result = Activator
                    .getDefault()
                    .getResourceManager()
                    .completeCollectionResourceCreationByRAML(
                            createWizard.getLoadRamlPage().getResource(),
                            resURI, resName);
            if (result)
                status = "Resource created.";
            else {
                status = "Failed to create resource.";
            }
        } catch (SimulatorException e) {
            status = "Failed to create resource.\n"
                    + Utility.getSimulatorErrorString(e, null);
        }
        createWizard.setStatus(status);
        return result;
    }
}
